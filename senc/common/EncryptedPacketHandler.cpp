/*********************************************************************
 * \file   EncryptedPacketHandler.cpp
 * \brief  Implementation of EncryptedPacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "EncryptedPacketHandler.hpp"

#include "SockUtils.hpp"

namespace senc
{
	EncryptedPacketHandler::EncryptedPacketHandler()
		: _powDist(utils::Random<utils::BigInt>::get_dist_below(Group::order())) { }
	
	std::pair<bool, std::string> EncryptedPacketHandler::establish_connection_client_side(utils::Socket& sock)
	{
		// send protocol version
		sock.send_connected_primitive(pkt::PROTOCOL_VERSION);

		// receive flag indicating whether protocol version is OK
		const bool isProtocolVersoinOK = sock.recv_connected_primitive<bool>();
		if (!isProtocolVersoinOK)
			return { false, "Bad protocol version" };

		try
		{
			// sample x and send g^x for key exchange
			const utils::BigInt x = _powDist();
			const Group gx = Group::generator().pow(x);
			SockUtils::send_ecgroup_elem(sock, gx);

			// receive gy for key exchange
			Group gy{};
			SockUtils::recv_ecgroup_elem(sock, gy);

			// compute g^xy and dereive key
			gy *= gx;
			_key = _kdf(gy);
		}
		catch (const std::exception& e)
		{
			return { false, std::string("Failed to exchange key: ") + e.what() };
		}

		return { true, "" }; // success
	}

	std::pair<bool, std::string> EncryptedPacketHandler::establish_connection_server_side(utils::Socket& sock)
	{
		// receive & check protocol version
		auto protocolVersion = sock.recv_connected_primitive<std::uint8_t>();
		if (protocolVersion != pkt::PROTOCOL_VERSION)
		{
			sock.send_connected_primitive(false); // bad protocol version
			return { false, "Bad protocol version" };
		}
		sock.send_connected_primitive(true); // protocol version OK

		try
		{
			// receive gx for key exchange
			Group gx{};
			SockUtils::recv_ecgroup_elem(sock, gx);

			// sample y and send gy for key exchange
			const utils::BigInt y = _powDist();
			const Group gy = Group::generator().pow(y);
			SockUtils::send_ecgroup_elem(sock, gy);

			// compute g^xy and dereive key
			gx *= gy;
			_key = _kdf(gx);
		}
		catch (const std::exception& e)
		{
			return { false, std::string("Failed to exchange key: ") + e.what() };
		}

		return { true, "" }; // success
	}
}

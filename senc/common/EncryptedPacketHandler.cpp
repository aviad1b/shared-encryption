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

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::ErrorResponse& packet)
	{
		utils::Buffer data(packet.msg.begin(), packet.msg.end());
		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);

		out.msg = std::string(data.begin(), data.end());
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::SignupRequest& packet)
	{
		utils::Buffer data{};

		data.insert(data.end(), packet.username.begin(), packet.username.end());
		data.insert(data.end(), packet.password.begin(), packet.password.end());

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::SignupRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);

		auto p = reinterpret_cast<const char*>(data.data());

		// read first string (up to null termination) into username,
		// and everything after that (up to next null termination) into password
		out.username = p;
		out.password = p + out.username.length() + 1;
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::SignupResponse& packet)
	{
		utils::Buffer data{};
		utils::append_primitive_bytes(data, packet.status);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::SignupResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);

		out.status = *reinterpret_cast<const pkt::SignupResponse::Status*>(data.data());
	}

	void EncryptedPacketHandler::send_encrypted_data(utils::Socket& sock, const utils::Buffer& data)
	{
		utils::enc::Ciphertext<Schema> encryptedData = _schema.encrypt(data, _key);
		const auto& [c1, c2] = encryptedData;
		sock.send_connected_primitive(static_cast<std::uint64_t>(c1.size()));
		sock.send_connected_primitive(static_cast<std::uint64_t>(c2.size()));
		sock.send_connected(c1);
		sock.send_connected(c2);
	}

	void EncryptedPacketHandler::recv_encrypted_data(utils::Socket& sock, utils::Buffer& out)
	{
		utils::enc::Ciphertext<Schema> encryptedData{};
		auto& [c1, c2] = encryptedData;

		const auto c1Size = sock.recv_connected_primitive<std::uint64_t>();
		c1.resize(c1Size);

		const auto c2Size = sock.recv_connected_primitive<std::uint64_t>();
		c2.resize(c2Size);

		sock.recv_connected_exact_into(c1);
		sock.recv_connected_exact_into(c2);

		out = _schema.decrypt(encryptedData, _key);
	}
}

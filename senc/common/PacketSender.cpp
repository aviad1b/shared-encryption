/*********************************************************************
 * \file   PacketSender.cpp
 * \brief  Implementation of PacketSender class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "PacketSender.hpp"

namespace senc
{
	void PacketSender::send_packet(utils::Socket& sock, const pkt::ErrorResponse& packet)
	{
		sock.send_connected_value(packet.msg);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::SignupRequest& packet)
	{
		sock.send_connected_value(packet.username);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::SignupResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::LoginRequest& packet)
	{
		sock.send_connected_value(packet.username);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::LoginResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::LogoutRequest& packet)
	{
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::LogoutResponse& packet)
	{
	}

	void PacketSender::send_big_int(utils::Socket& sock, const utils::BigInt& value)
	{
		sock.send_connected_value(static_cast<std::uint64_t>(value.MinEncodedSize()));

		utils::Buffer buff(value.MinEncodedSize());
		value.Encode(buff.data(), buff.size());
		sock.send_connected(buff);
	}
}

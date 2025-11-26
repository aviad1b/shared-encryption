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
}

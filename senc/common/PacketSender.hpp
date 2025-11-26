/*********************************************************************
 * \file   PacketSender.hpp
 * \brief  Header of PacketSender class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../utils/Socket.hpp"
#include "packets.hpp"

namespace senc
{
	class PacketSender
	{
	public:
		PacketSender() = delete;

		static void send_packet(utils::Socket& sock, const pkt::ErrorResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::SignupRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::SignupResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::LoginRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::LoginResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::LogoutRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::LogoutResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::MakeUserSetRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::MakeUserSetResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::GetUserSetsRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::GetUserSetsResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::GetMembersRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::GetMembersResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::DecryptRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::DecryptResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::UpdateRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::UpdateResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet);

		static void send_packet(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet);
		static void send_packet(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet);
	};
}

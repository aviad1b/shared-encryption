/*********************************************************************
 * \file   PacketSender.hpp
 * \brief  Contains abstract class PacketSender.
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
		virtual ~PacketSender() { }

		virtual void send_response(utils::Socket& sock, const pkt::ErrorResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::SignupRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::SignupResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::LoginRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::LoginResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::LogoutRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::LogoutResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::MakeUserSetRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::MakeUserSetResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::GetUserSetsRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::GetUserSetsResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::GetMembersRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::GetMembersResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::DecryptRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::DecryptResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::UpdateRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::UpdateResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet) = 0;

		virtual void send_request(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet) = 0;
		virtual void send_response(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet) = 0;
	};
}

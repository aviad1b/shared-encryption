/*********************************************************************
 * \file   PacketReceiver.hpp
 * \brief  Contains abstract class PacketReceiver.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../utils/Socket.hpp"
#include "packets.hpp"

namespace senc
{
	class PacketReceiver
	{
	public:
		virtual ~PacketReceiver() { }

		virtual void recv_response(utils::Socket& sock, pkt::ErrorResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::SignupRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::SignupResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::LoginRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::LoginResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::LogoutRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::LogoutResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::MakeUserSetRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::MakeUserSetResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::GetUserSetsRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::GetUserSetsResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::GetMembersRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::GetMembersResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::DecryptRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::DecryptResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::UpdateRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::UpdateResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::DecryptParticipateRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::DecryptParticipateResponse& out) = 0;

		virtual void recv_request(utils::Socket& sock, pkt::SendDecryptionPartRequest& out) = 0;
		virtual void recv_response(utils::Socket& sock, pkt::SendDecryptionPartResponse& out) = 0;

		template <typename T>
		inline T recv_request(utils::Socket& sock)
		{
			T req{};
			this->recv_request(sock, req);
			return req;
		}

		template <typename T>
		inline T recv_response(utils::Socket& sock)
		{
			T resp{};
			this->recv_response(sock, resp);
			return resp;
		}
	};
}

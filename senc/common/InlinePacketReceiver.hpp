/*********************************************************************
 * \file   PacketReceiver.hpp
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
	class InlinePacketReceiver
	{
	public:
		InlinePacketReceiver() = delete;

		template <typename T>
		static inline T recv_request(utils::Socket& sock)
		{
			T req{};
			recv_request(sock, req);
			return req;
		}

		template <typename T>
		static inline T recv_response(utils::Socket& sock)
		{
			T resp{};
			recv_response(sock, resp);
			return resp;
		}

		void recv_response(utils::Socket& sock, pkt::ErrorResponse& out);

		void recv_request(utils::Socket& sock, pkt::SignupRequest& out);
		void recv_response(utils::Socket& sock, pkt::SignupResponse& out);

		void recv_request(utils::Socket& sock, pkt::LoginRequest& out);
		void recv_response(utils::Socket& sock, pkt::LoginResponse& out);

		void recv_request(utils::Socket& sock, pkt::LogoutRequest& out);
		void recv_response(utils::Socket& sock, pkt::LogoutResponse& out);

		void recv_request(utils::Socket& sock, pkt::MakeUserSetRequest& out);
		void recv_response(utils::Socket& sock, pkt::MakeUserSetResponse& out);

		void recv_request(utils::Socket& sock, pkt::GetUserSetsRequest& out);
		void recv_response(utils::Socket& sock, pkt::GetUserSetsResponse& out);

		void recv_request(utils::Socket& sock, pkt::GetMembersRequest& out);
		void recv_response(utils::Socket& sock, pkt::GetMembersResponse& out);

		void recv_request(utils::Socket& sock, pkt::DecryptRequest& out);
		void recv_response(utils::Socket& sock, pkt::DecryptResponse& out);

		void recv_request(utils::Socket& sock, pkt::UpdateRequest& out);
		void recv_response(utils::Socket& sock, pkt::UpdateResponse& out);

		void recv_request(utils::Socket& sock, pkt::DecryptParticipateRequest& out);
		void recv_response(utils::Socket& sock, pkt::DecryptParticipateResponse& out);

		void recv_request(utils::Socket& sock, pkt::SendDecryptionPartRequest& out);
		void recv_response(utils::Socket& sock, pkt::SendDecryptionPartResponse& out);

	private:
		static void recv_big_int(utils::Socket& sock, utils::BigInt& out);

		static void recv_pub_key(utils::Socket& sock, PubKey& out);
		static void recv_priv_key_shard(utils::Socket& sock, PrivKeyShard& out);

		static void recv_ciphertext(utils::Socket& sock, Ciphertext& out);
		static void recv_decryption_part(utils::Socket& sock, DecryptionPart& out);

		static void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsOwnerRecord& out);
		static void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsMemberRecord& out);
		static void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::ToDecryptRecord& out);
		static void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::FinishedDecryptionsRecord& out);
	};
}

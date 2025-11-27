/*********************************************************************
 * \file   InlinePacketSender.hpp
 * \brief  Header of InlinePacketSender class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../utils/ECGroup.hpp"
#include "../utils/Socket.hpp"
#include "packets.hpp"

namespace senc
{
	class InlinePacketSender
	{
	public:
		void send_response(utils::Socket& sock, const pkt::ErrorResponse& packet);

		void send_request(utils::Socket& sock, const pkt::SignupRequest& packet);
		void send_response(utils::Socket& sock, const pkt::SignupResponse& packet);

		void send_request(utils::Socket& sock, const pkt::LoginRequest& packet);
		void send_response(utils::Socket& sock, const pkt::LoginResponse& packet);

		void send_request(utils::Socket& sock, const pkt::LogoutRequest& packet);
		void send_response(utils::Socket& sock, const pkt::LogoutResponse& packet);

		void send_request(utils::Socket& sock, const pkt::MakeUserSetRequest& packet);
		void send_response(utils::Socket& sock, const pkt::MakeUserSetResponse& packet);

		void send_request(utils::Socket& sock, const pkt::GetUserSetsRequest& packet);
		void send_response(utils::Socket& sock, const pkt::GetUserSetsResponse& packet);

		void send_request(utils::Socket& sock, const pkt::GetMembersRequest& packet);
		void send_response(utils::Socket& sock, const pkt::GetMembersResponse& packet);

		void send_request(utils::Socket& sock, const pkt::DecryptRequest& packet);
		void send_response(utils::Socket& sock, const pkt::DecryptResponse& packet);

		void send_request(utils::Socket& sock, const pkt::UpdateRequest& packet);
		void send_response(utils::Socket& sock, const pkt::UpdateResponse& packet);

		void send_request(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet);
		void send_response(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet);

		void send_request(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet);
		void send_response(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet);

	private:
		void send_big_int(utils::Socket& sock, const utils::BigInt& value);

		void send_pub_key(utils::Socket& sock, const PubKey& pubKey);
		void send_priv_key_shard(utils::Socket& sock, const PrivKeyShard& shard);

		void send_ciphertext(utils::Socket& sock, const Ciphertext& ciphertext);
		void send_decryption_part(utils::Socket& sock, const DecryptionPart& part);

		void send_update_record(utils::Socket& sock, const pkt::UpdateResponse::AddedAsOwnerRecord& record);
		void send_update_record(utils::Socket& sock, const pkt::UpdateResponse::AddedAsMemberRecord& record);
		void send_update_record(utils::Socket& sock, const pkt::UpdateResponse::ToDecryptRecord& record);
		void send_update_record(utils::Socket& sock, const pkt::UpdateResponse::FinishedDecryptionsRecord& record);
	};
}

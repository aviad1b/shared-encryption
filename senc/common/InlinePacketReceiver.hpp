/*********************************************************************
 * \file   InlinePacketReceiver.hpp
 * \brief  Header of InlinePacketSender class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "PacketReceiver.hpp"
#include <optional>

namespace senc
{
	class InlinePacketReceiver : public PacketReceiver
	{
	public:
		using Self = InlinePacketReceiver;
		using Base = PacketReceiver;

		void recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::SignupRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::SignupResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::LoginRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::LoginResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::LogoutRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::LogoutResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::MakeUserSetRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::MakeUserSetResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::GetUserSetsRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::GetUserSetsResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::GetMembersRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::GetMembersResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::DecryptRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::DecryptResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::UpdateRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::UpdateResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::DecryptParticipateRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::DecryptParticipateResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::SendDecryptionPartRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::SendDecryptionPartResponse& out) override;

	private:
		static bool recv_big_int(utils::Socket& sock, utils::BigInt& out);
		static void recv_ecgroup_elem(utils::Socket& sock, utils::ECGroup& out);

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

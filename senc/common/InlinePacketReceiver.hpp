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

		bool recv_connection_request(utils::Socket& sock) override;

		bool recv_connection_response(utils::Socket& sock) override;

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
		bool recv_big_int(utils::Socket& sock, utils::BigInt& out);
		void recv_ecgroup_elem(utils::Socket& sock, utils::ECGroup& out);

		void recv_pub_key(utils::Socket& sock, PubKey& out);
		void recv_priv_key_shard_id(utils::Socket& sock, PrivKeyShardID& out);
		void recv_priv_key_shard(utils::Socket& sock, PrivKeyShard& out);

		void recv_ciphertext(utils::Socket& sock, Ciphertext& out);
		void recv_decryption_part(utils::Socket& sock, DecryptionPart& out);

		void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsOwnerRecord& out);
		void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsMemberRecord& out);
		void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::ToDecryptRecord& out);
		void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::FinishedDecryptionsRecord& out);
	};
}

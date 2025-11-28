/*********************************************************************
 * \file   InlinePacketSender.hpp
 * \brief  Header of InlinePacketSender class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "PacketSender.hpp"

namespace senc
{
	class InlinePacketSender : public PacketSender
	{
	public:
		using Self = InlinePacketSender;
		using Base = PacketSender;

		void send_response_data(utils::Socket& sock, const pkt::ErrorResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::SignupRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::SignupResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::LoginRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::LoginResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::LogoutRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::LogoutResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::MakeUserSetRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::MakeUserSetResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::GetUserSetsRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::GetUserSetsResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::GetMembersRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::GetMembersResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::DecryptRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::DecryptResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::UpdateRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::UpdateResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet) override;

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

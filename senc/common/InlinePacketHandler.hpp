/*********************************************************************
 * \file   InlinePacketHandler.hpp
 * \brief  Header of InlinePacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "ConnEstablishException.hpp"
#include "PacketHandler.hpp"

namespace senc
{
	/**
	 * @class senc::InlinePacketHandler
	 * @brief Inline implementation of `senc::PacketHandler`.
	 */
	class InlinePacketHandler : public PacketHandler
	{
	public:
		using Self = InlinePacketHandler;
		using Base = PacketHandler;

		InlinePacketHandler(Self&&) = default;

		/**
		 * @brief Gets handler instance for server side.
		 * @param sock Socket to send and receive packets through.
		 * @throw ConnEstablishException If failed to establish connection.
		 */
		static Self server(utils::Socket& sock);

		/**
		 * @brief Gets handler instance for client side.
		 * @param sock Socket to send and receive packets through.
		 * @throw ConnEstablishException If failed to establish connection.
		 */
		static Self client(utils::Socket& sock);

		void send_response_data(const pkt::ErrorResponse& packet) override;
		void recv_response_data(pkt::ErrorResponse& out) override;

		void send_request_data(const pkt::SignupRequest& packet) override;
		void recv_request_data(pkt::SignupRequest& out) override;

		void send_response_data(const pkt::SignupResponse& packet) override;
		void recv_response_data(pkt::SignupResponse& out) override;

		void send_request_data(const pkt::LoginRequest& packet) override;
		void recv_request_data(pkt::LoginRequest& out) override;

		void send_response_data(const pkt::LoginResponse& packet) override;
		void recv_response_data(pkt::LoginResponse& out) override;

		void send_request_data(const pkt::LogoutRequest& packet) override;
		void recv_request_data(pkt::LogoutRequest& out) override;

		void send_response_data(const pkt::LogoutResponse& packet) override;
		void recv_response_data(pkt::LogoutResponse& out) override;

		void send_request_data(const pkt::MakeUserSetRequest& packet) override;
		void recv_request_data(pkt::MakeUserSetRequest& out) override;

		void send_response_data(const pkt::MakeUserSetResponse& packet) override;
		void recv_response_data(pkt::MakeUserSetResponse& out) override;

		void send_request_data(const pkt::GetUserSetsRequest& packet) override;
		void recv_request_data(pkt::GetUserSetsRequest& out) override;

		void send_response_data(const pkt::GetUserSetsResponse& packet) override;
		void recv_response_data(pkt::GetUserSetsResponse& out) override;

		void send_request_data(const pkt::GetMembersRequest& packet) override;
		void recv_request_data(pkt::GetMembersRequest& out) override;

		void send_response_data(const pkt::GetMembersResponse& packet) override;
		void recv_response_data(pkt::GetMembersResponse& out) override;

		void send_request_data(const pkt::DecryptRequest& packet) override;
		void recv_request_data(pkt::DecryptRequest& out) override;

		void send_response_data(const pkt::DecryptResponse& packet) override;
		void recv_response_data(pkt::DecryptResponse& out) override;

		void send_request_data(const pkt::UpdateRequest& packet) override;
		void recv_request_data(pkt::UpdateRequest& out) override;

		void send_response_data(const pkt::UpdateResponse& packet) override;
		void recv_response_data(pkt::UpdateResponse& out) override;

		void send_request_data(const pkt::DecryptParticipateRequest& packet) override;
		void recv_request_data(pkt::DecryptParticipateRequest& out) override;

		void send_response_data(const pkt::DecryptParticipateResponse& packet) override;
		void recv_response_data(pkt::DecryptParticipateResponse& out) override;

		void send_request_data(const pkt::SendDecryptionPartRequest& packet) override;
		void recv_request_data(pkt::SendDecryptionPartRequest& out) override;

		void send_response_data(const pkt::SendDecryptionPartResponse& packet) override;
		void recv_response_data(pkt::SendDecryptionPartResponse& out) override;

	private:
		void send_pub_key(const PubKey& pubKey);
		void recv_pub_key(PubKey& out);

		void send_priv_key_shard_id(const PrivKeyShardID& shardID);
		void recv_priv_key_shard_id(PrivKeyShardID& out);

		void send_priv_key_shard(const PrivKeyShard& shard);
		void recv_priv_key_shard(PrivKeyShard& out);

		void send_ciphertext(const Ciphertext& ciphertext);
		void recv_ciphertext(Ciphertext& out);

		void send_decryption_part(const DecryptionPart& part);
		void recv_decryption_part(DecryptionPart& out);

		void send_update_record(const pkt::UpdateResponse::AddedAsOwnerRecord& record);
		void recv_update_record(pkt::UpdateResponse::AddedAsOwnerRecord& out);

		void send_update_record(const pkt::UpdateResponse::AddedAsMemberRecord& record);
		void recv_update_record(pkt::UpdateResponse::AddedAsMemberRecord& out);

		void send_update_record(const pkt::UpdateResponse::ToDecryptRecord& record);
		void recv_update_record(pkt::UpdateResponse::ToDecryptRecord& out);

		void send_update_record(const pkt::UpdateResponse::FinishedDecryptionsRecord& record);
		void recv_update_record(pkt::UpdateResponse::FinishedDecryptionsRecord& out);

		private:
			InlinePacketHandler(utils::Socket& sock);
	};

	static_assert(PacketHandlerImpl<InlinePacketHandler>);
}

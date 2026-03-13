/*********************************************************************
 * \file   InlinePacketHandler.hpp
 * \brief  Header of InlinePacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "PlainPacketHandlerSyncData.hpp"
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

		const IPacketHandlerSyncData& get_sync_data() const override;

		void send_response(const pkt::ErrorResponse& packet) override;
		void recv_response_data(pkt::ErrorResponse& out) override;

		void send_request(const pkt::SignupRequest& packet) override;
		void recv_request_data(pkt::SignupRequest& out) override;

		void send_response(const pkt::SignupResponse& packet) override;
		void recv_response_data(pkt::SignupResponse& out) override;

		void send_request(const pkt::LoginRequest& packet) override;
		void recv_request_data(pkt::LoginRequest& out) override;

		void send_response(const pkt::LoginResponse& packet) override;
		void recv_response_data(pkt::LoginResponse& out) override;

		void send_request(const pkt::LogoutRequest& packet) override;
		void recv_request_data(pkt::LogoutRequest& out) override;

		void send_response(const pkt::LogoutResponse& packet) override;
		void recv_response_data(pkt::LogoutResponse& out) override;

		void send_request(const pkt::MakeUserSetRequest& packet) override;
		void recv_request_data(pkt::MakeUserSetRequest& out) override;

		void send_response(const pkt::MakeUserSetResponse& packet) override;
		void recv_response_data(pkt::MakeUserSetResponse& out) override;

		void send_request(const pkt::GetUserSetsRequest& packet) override;
		void recv_request_data(pkt::GetUserSetsRequest& out) override;

		void send_response(const pkt::GetUserSetsResponse& packet) override;
		void recv_response_data(pkt::GetUserSetsResponse& out) override;

		void send_request(const pkt::GetMembersRequest& packet) override;
		void recv_request_data(pkt::GetMembersRequest& out) override;

		void send_response(const pkt::GetMembersResponse& packet) override;
		void recv_response_data(pkt::GetMembersResponse& out) override;

		void send_request(const pkt::DecryptRequest& packet) override;
		void recv_request_data(pkt::DecryptRequest& out) override;

		void send_response(const pkt::DecryptResponse& packet) override;
		void recv_response_data(pkt::DecryptResponse& out) override;

		void send_request(const pkt::UpdateRequest& packet) override;
		void recv_request_data(pkt::UpdateRequest& out) override;

		void send_response(const pkt::UpdateResponse& packet) override;
		void recv_response_data(pkt::UpdateResponse& out) override;

		void send_request(const pkt::DecryptParticipateRequest& packet) override;
		void recv_request_data(pkt::DecryptParticipateRequest& out) override;

		void send_response(const pkt::DecryptParticipateResponse& packet) override;
		void recv_response_data(pkt::DecryptParticipateResponse& out) override;

		void send_request(const pkt::SendDecryptionPartRequest& packet) override;
		void recv_request_data(pkt::SendDecryptionPartRequest& out) override;

		void send_response(const pkt::SendDecryptionPartResponse& packet) override;
		void recv_response_data(pkt::SendDecryptionPartResponse& out) override;

		virtual void send_request(const pkt::UserSearchRequest& packet) override;
		virtual void recv_request_data(pkt::UserSearchRequest& out) override;

		virtual void send_response(const pkt::UserSearchResponse& packet) override;
		virtual void recv_response_data(pkt::UserSearchResponse& out) override;

	private:
		PlainPacketHandlerSyncData _syncData;

		InlinePacketHandler(utils::Socket& sock);

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

		void send_update_record(const pkt::UpdateResponse::OnLookupRecord& record);
		void recv_update_record(pkt::UpdateResponse::OnLookupRecord& out);

		void send_update_record(const pkt::UpdateResponse::ToDecryptRecord& record);
		void recv_update_record(pkt::UpdateResponse::ToDecryptRecord& out);

		void send_update_record(const pkt::UpdateResponse::FinishedDecryptionsRecord& record);
		void recv_update_record(pkt::UpdateResponse::FinishedDecryptionsRecord& out);

		void send_update_record(const pkt::UpdateResponse::ToEvolveRecord& record);
		void recv_update_record(pkt::UpdateResponse::ToEvolveRecord& out);
	};

	static_assert(PacketHandlerImpl<InlinePacketHandler>);
}

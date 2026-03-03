/*********************************************************************
 * \file   EncryptedPacketHandler.hpp
 * \brief  Header of EncryptedPacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "KeyedPacketHandlerSyncData.hpp"
#include "ConnEstablishException.hpp"
#include "../utils/enc/ECHKDF1L.hpp"
#include "../utils/enc/AES1L.hpp"
#include "../utils/Random.hpp"
#include "PacketHandler.hpp"

namespace senc
{
	/**
	 * @class senc::EncryptedPacketHandler
	 * @brief Encrypted implementation of `senc::PacketHandler`.
	 * @note For encryption efficiency, this class heavily relies on the assumption that 
	 *       `recv_code` is always and only called before `recv_request_data`/`recv_response_data`, and
	 *       `recv_request_data`/`recv_response_data` is always and only called after `recv_code`.
	 */
	class EncryptedPacketHandler : public PacketHandler
	{
	public:
		using Self = EncryptedPacketHandler;
		using Base = PacketHandler;
		using Schema = utils::enc::AES1L;
		using Key = utils::enc::Key<Schema>;
		using Group = utils::ECGroup;
		using KDF = utils::enc::ECHKDF1L;

		EncryptedPacketHandler(Self&&) = default;

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

		pkt::Code recv_code() override;

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

	protected:
		EncryptedPacketHandler(utils::Socket& sock);

	private:
		KeyedPacketHandlerSyncData<Key> _syncData;
		utils::BytesView _buffView;
		utils::Buffer _buff;
		Schema _schema;
		KDF _kdf;

		static utils::BigInt sample_pow()
		{
			static thread_local utils::Distribution<utils::BigInt> powDist(
				utils::Random<utils::BigInt>::get_dist_below(Group::order())
			);
			return powDist();
		}

		/**
		 * @typedef encdata_size_t
		 * @brief Primitive used for size of encrypted packet data.
		 */
		using encdata_size_t = std::uint64_t;

		/**
		 * @brief Maximum size of encrypted packet data.
		 */
		static constexpr std::size_t MAX_ENCDATA_SIZE = std::numeric_limits<encdata_size_t>::max();

		void send_encrypted_data(const utils::Buffer& data);
		
		void recv_encrypted_data(); // receives into `buff`.

		void write_big_int(utils::Buffer& out, const std::optional<utils::BigInt>& value);
		utils::BytesView::iterator read_big_int(std::optional<utils::BigInt>& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_ecgroup_elem(utils::Buffer& out, const utils::ECGroup& elem);
		utils::BytesView::iterator read_ecgroup_elem(utils::ECGroup& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_pub_key(utils::Buffer& out, const PubKey& pubKey);
		utils::BytesView::iterator read_pub_key(PubKey& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_priv_key_shard_id(utils::Buffer& out, const PrivKeyShardID& shardID);
		utils::BytesView::iterator read_priv_key_shard_id(PrivKeyShardID& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_priv_key_shard(utils::Buffer& out, const PrivKeyShard& shard);
		utils::BytesView::iterator read_priv_key_shard(PrivKeyShard& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_ciphertext(utils::Buffer& out, const Ciphertext& ciphertext);
		utils::BytesView::iterator read_ciphertext(Ciphertext& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_decryption_part(utils::Buffer& out, const DecryptionPart& part);
		utils::BytesView::iterator read_decryption_part(DecryptionPart& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::AddedAsOwnerRecord& record);
		utils::BytesView::iterator read_update_record(pkt::UpdateResponse::AddedAsOwnerRecord& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::AddedAsMemberRecord& record);
		utils::BytesView::iterator read_update_record(pkt::UpdateResponse::AddedAsMemberRecord& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::OnLookupRecord& record);
		utils::BytesView::iterator read_update_record(pkt::UpdateResponse::OnLookupRecord& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::ToDecryptRecord& record);
		utils::BytesView::iterator read_update_record(pkt::UpdateResponse::ToDecryptRecord& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::FinishedDecryptionsRecord& record);
		utils::BytesView::iterator read_update_record(pkt::UpdateResponse::FinishedDecryptionsRecord& out,
			utils::BytesView::iterator it, utils::BytesView::iterator end);
	};

	static_assert(PacketHandlerImpl<EncryptedPacketHandler>);
}

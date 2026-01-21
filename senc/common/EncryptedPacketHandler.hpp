/*********************************************************************
 * \file   EncryptedPacketHandler.hpp
 * \brief  Header of EncryptedPacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "ConnEstablishException.hpp"
#include "../utils/enc/ECHKDF1L.hpp"
#include "../utils/enc/AES1L.hpp"
#include "../utils/Random.hpp"
#include "PacketHandler.hpp"

namespace senc
{
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

		bool validate_synchronization(const Base* other) const override;

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

	protected:
		EncryptedPacketHandler(utils::Socket& sock);

	private:
		Schema _schema;
		KDF _kdf;
		Key _key;

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
		
		void recv_encrypted_data(utils::Buffer& out);

		void write_big_int(utils::Buffer& out, const std::optional<utils::BigInt>& value);
		utils::Buffer::iterator read_big_int(std::optional<utils::BigInt>& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_ecgroup_elem(utils::Buffer& out, const utils::ECGroup& elem);
		utils::Buffer::iterator read_ecgroup_elem(utils::ECGroup& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_pub_key(utils::Buffer& out, const PubKey& pubKey);
		utils::Buffer::iterator read_pub_key(PubKey& out, 
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_priv_key_shard_id(utils::Buffer& out, const PrivKeyShardID& shardID);
		utils::Buffer::iterator read_priv_key_shard_id(PrivKeyShardID& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_priv_key_shard(utils::Buffer& out, const PrivKeyShard& shard);
		utils::Buffer::iterator read_priv_key_shard(PrivKeyShard& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_ciphertext(utils::Buffer& out, const Ciphertext& ciphertext);
		utils::Buffer::iterator read_ciphertext(Ciphertext& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_decryption_part(utils::Buffer& out, const DecryptionPart& part);
		utils::Buffer::iterator read_decryption_part(DecryptionPart& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::AddedAsOwnerRecord& record);
		utils::Buffer::iterator read_update_record(pkt::UpdateResponse::AddedAsOwnerRecord& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::AddedAsMemberRecord& record);
		utils::Buffer::iterator read_update_record(pkt::UpdateResponse::AddedAsMemberRecord& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::ToDecryptRecord& record);
		utils::Buffer::iterator read_update_record(pkt::UpdateResponse::ToDecryptRecord& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);

		void write_update_record(utils::Buffer& out, const pkt::UpdateResponse::FinishedDecryptionsRecord& record);
		utils::Buffer::iterator read_update_record(pkt::UpdateResponse::FinishedDecryptionsRecord& out,
			utils::Buffer::iterator it, utils::Buffer::iterator end);
	};

	static_assert(PacketHandlerImpl<EncryptedPacketHandler>);
}

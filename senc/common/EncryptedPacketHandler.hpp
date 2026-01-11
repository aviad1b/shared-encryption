/*********************************************************************
 * \file   EncryptedPacketHandler.hpp
 * \brief  Header of EncryptedPacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

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
		using Group = utils::ECGroup;
		using KDF = utils::enc::ECHKDF1L;

		EncryptedPacketHandler();

		std::pair<bool, std::string> establish_connection_client_side(utils::Socket& sock) override;
		std::pair<bool, std::string> establish_connection_server_side(utils::Socket& sock) override;

		void send_response_data(utils::Socket& sock, const pkt::ErrorResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::SignupRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::SignupRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::SignupResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::SignupResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::LoginRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::LoginRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::LoginResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::LoginResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::LogoutRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::LogoutRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::LogoutResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::LogoutResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::MakeUserSetRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::MakeUserSetRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::MakeUserSetResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::MakeUserSetResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::GetUserSetsRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::GetUserSetsRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::GetUserSetsResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::GetUserSetsResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::GetMembersRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::GetMembersRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::GetMembersResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::GetMembersResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::DecryptRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::DecryptRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::DecryptResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::DecryptResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::UpdateRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::UpdateRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::UpdateResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::UpdateResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::DecryptParticipateRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::DecryptParticipateResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::SendDecryptionPartRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::SendDecryptionPartResponse& out) override;

	private:
		utils::Distribution<utils::BigInt> _powDist;
		Schema _schema;
		KDF _kdf;
		utils::enc::Key<Schema> _key;

		void send_encrypted_data(utils::Socket& sock, const utils::Buffer& data);
		
		void recv_encrypted_data(utils::Socket& sock, utils::Buffer& out);

		void write_ecgroup_elem(utils::Buffer& out, const utils::ECGroup& pubKey);
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
}

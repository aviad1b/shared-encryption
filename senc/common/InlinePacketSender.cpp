/*********************************************************************
 * \file   InlinePacketSender.cpp
 * \brief  Implementation of InlinePacketSender class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "InlinePacketSender.hpp"

namespace senc
{
	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::ErrorResponse& packet)
	{
		sock.send_connected_value(packet.msg);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::SignupRequest& packet)
	{
		sock.send_connected_value(packet.username);
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::SignupResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::LoginRequest& packet)
	{
		sock.send_connected_value(packet.username);
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::LoginResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::LogoutRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::LogoutResponse& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::MakeUserSetRequest& packet)
	{
		sock.send_connected_value(packet.owners_threshold);
		sock.send_connected_value(packet.reg_members_threshold);
		sock.send_connected_value(static_cast<member_count_t>(packet.owners.size()));
		sock.send_connected_value(static_cast<member_count_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			sock.send_connected_value(owner);
		for (const auto& regMember : packet.reg_members)
			sock.send_connected_value(regMember);
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::MakeUserSetResponse& packet)
	{
		sock.send_connected_value(packet.user_set_id);
		send_pub_key(sock, packet.pub_key1);
		send_pub_key(sock, packet.pub_key2);
		send_priv_key_shard(sock, packet.priv_key1_shard);
		send_priv_key_shard(sock, packet.priv_key2_shard);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::GetUserSetsRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::GetUserSetsResponse& packet)
	{
		sock.send_connected_value(static_cast<userset_count_t>(packet.user_sets_ids.size()));
		for (const auto& userSetID : packet.user_sets_ids)
			sock.send_connected_value(userSetID);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::GetMembersRequest& packet)
	{
		sock.send_connected_value(packet.user_set_id);
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::GetMembersResponse& packet)
	{
		sock.send_connected_value(static_cast<member_count_t>(packet.owners.size()));
		sock.send_connected_value(static_cast<member_count_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			sock.send_connected_value(owner);
		for (const auto& reg_member : packet.reg_members)
			sock.send_connected_value(reg_member);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::DecryptRequest& packet)
	{
		sock.send_connected_value(packet.user_set_id);
		send_ciphertext(sock, packet.ciphertext);
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::DecryptResponse& packet)
	{
		sock.send_connected_value(packet.op_id);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::UpdateRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::UpdateResponse& packet)
	{
		// send vector lengths
		sock.send_connected_value(static_cast<userset_count_t>(packet.added_as_owner.size()));
		sock.send_connected_value(static_cast<userset_count_t>(packet.added_as_reg_member.size()));
		sock.send_connected_value(static_cast<lookup_count_t>(packet.on_lookup.size()));
		sock.send_connected_value(static_cast<pending_count_t>(packet.to_decrypt.size()));
		sock.send_connected_value(static_cast<res_count_t>(packet.finished_decryptions.size()));

		// send added_as_owner records
		for (const auto& record : packet.added_as_owner)
			send_update_record(sock, record);

		// send added_as_reg_member records
		for (const auto& record : packet.added_as_reg_member)
			send_update_record(sock, record);
		
		// send on_lookup records
		for (const auto& record : packet.on_lookup)
			sock.send_connected_value(record);

		// send to_decrypt records
		for (const auto& record : packet.to_decrypt)
			send_update_record(sock, record);

		// send finished_decryptions records
		for (const auto& record : packet.finished_decryptions)
			send_update_record(sock, record);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet)
	{
		sock.send_connected_value(packet.op_id);
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void InlinePacketSender::send_request_data(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet)
	{
		sock.send_connected_value(packet.op_id);
		send_decryption_part(sock, packet.decryption_part);
	}

	void InlinePacketSender::send_response_data(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketSender::send_big_int(utils::Socket& sock, const std::optional<utils::BigInt>& value)
	{
		if (!value.has_value())
		{
			sock.send_connected_value(static_cast<bigint_size_t>(0));
			return;
		}
		
		sock.send_connected_value(static_cast<bigint_size_t>(value->MinEncodedSize()));

		utils::Buffer buff(value->MinEncodedSize());
		value->Encode(buff.data(), buff.size());
		sock.send_connected(buff);
	}

	void InlinePacketSender::send_ecgroup_elem(utils::Socket& sock, const utils::ECGroup& elem)
	{
		// if x is sent as nullopt then elem is identity (and y isn't sent)
		if (elem.is_identity())
		{
			send_big_int(sock, std::nullopt);
			return;
		}
		send_big_int(sock, elem.x());
		send_big_int(sock, elem.y());
	}

	void InlinePacketSender::send_pub_key(utils::Socket& sock, const PubKey& pubKey)
	{
		send_ecgroup_elem(sock, pubKey);
	}

	void InlinePacketSender::send_priv_key_shard(utils::Socket& sock, const PrivKeyShard& shard)
	{
		sock.send_connected_value(shard.first);
		send_big_int(sock, static_cast<const utils::BigInt&>(shard.second));
	}

	void InlinePacketSender::send_ciphertext(utils::Socket& sock, const Ciphertext& ciphertext)
	{
		const auto& [c1, c2, c3] = ciphertext;
		const auto& [c3a, c3b] = c3;

		send_ecgroup_elem(sock, c1);
		send_ecgroup_elem(sock, c2);
		
		sock.send_connected_value(static_cast<buffer_size_t>(c3a.size()));
		sock.send_connected_value(static_cast<buffer_size_t>(c3b.size()));
		sock.send_connected_value(c3);
	}

	void InlinePacketSender::send_decryption_part(utils::Socket& sock, const DecryptionPart& part)
	{
		send_ecgroup_elem(sock, part);
	}

	void InlinePacketSender::send_update_record(utils::Socket& sock, const pkt::UpdateResponse::AddedAsOwnerRecord& record)
	{
		send_update_record(
			sock,
			reinterpret_cast<const pkt::UpdateResponse::AddedAsMemberRecord&>(record)
		);
		send_priv_key_shard(sock, record.priv_key2_shard);
	}

	void InlinePacketSender::send_update_record(utils::Socket& sock, const pkt::UpdateResponse::AddedAsMemberRecord& record)
	{
		sock.send_connected_value(record.user_set_id);
		send_pub_key(sock, record.pub_key1);
		send_pub_key(sock, record.pub_key2);
		send_priv_key_shard(sock, record.priv_key1_shard);
	}

	void InlinePacketSender::send_update_record(utils::Socket& sock, const pkt::UpdateResponse::ToDecryptRecord& record)
	{
		sock.send_connected_value(record.op_id);
		send_ciphertext(sock, record.ciphertext);
		sock.send_connected_value(static_cast<member_count_t>(record.shards_ids.size()));
		for (const auto& shardID : record.shards_ids)
			sock.send_connected_value(shardID);
	}

	void InlinePacketSender::send_update_record(utils::Socket& sock, const pkt::UpdateResponse::FinishedDecryptionsRecord& record)
	{
		sock.send_connected_value(static_cast<member_count_t>(record.parts1.size()));
		sock.send_connected_value(static_cast<member_count_t>(record.parts2.size()));
		sock.send_connected_value(record.op_id);
		for (const auto& part : record.parts1)
			send_decryption_part(sock, part);
		for (const auto& part : record.parts2)
			send_decryption_part(sock, part);
	}
}

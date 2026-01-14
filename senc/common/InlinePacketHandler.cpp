/*********************************************************************
 * \file   InlinePacketHandler.cpp
 * \brief  Implementation of InlinePacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "InlinePacketHandler.hpp"

#include "SockUtils.hpp"

namespace senc
{
	std::pair<bool, std::string> InlinePacketHandler::establish_connection_client_side(utils::Socket& sock)
	{
		// send protocol version
		sock.send_connected_primitive(pkt::PROTOCOL_VERSION);

		// receive flag indicating whether protocol version is OK
		const bool isProtocolVersoinOK = sock.recv_connected_primitive<bool>();
		if (!isProtocolVersoinOK)
			return { false, "Bad protocol version" };
		return { true, "" }; // success
	}

	std::pair<bool, std::string> InlinePacketHandler::establish_connection_server_side(utils::Socket& sock)
	{
		// receive & check protocol version
		auto protocolVersion = sock.recv_connected_primitive<std::uint8_t>();
		if (protocolVersion != pkt::PROTOCOL_VERSION)
		{
			sock.send_connected_primitive(false); // bad protocol version
			return { false, "Bad protocol version" };
		}
		sock.send_connected_primitive(true); // protocol version OK
		
		return { true, "" }; // success
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::ErrorResponse& packet)
	{
		sock.send_connected_value(packet.msg);
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out)
	{
		sock.recv_connected_value(out.msg);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::SignupRequest& packet)
	{
		sock.send_connected_value(packet.username);
		sock.send_connected_value(packet.password);
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::SignupRequest& out)
	{
		sock.recv_connected_value(out.username);
		sock.recv_connected_value(out.password);
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::SignupResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::SignupResponse& out)
	{
		sock.recv_connected_value(out.status);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::LoginRequest& packet)
	{
		sock.send_connected_value(packet.username);
		sock.send_connected_value(packet.password);
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::LoginRequest& out)
	{
		sock.recv_connected_value(out.username);
		sock.recv_connected_value(out.password);
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::LoginResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::LoginResponse& out)
	{
		sock.recv_connected_value(out.status);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::LogoutRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::LogoutRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::LogoutResponse& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::LogoutResponse& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::MakeUserSetRequest& packet)
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

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::MakeUserSetRequest& out)
	{
		sock.recv_connected_value(out.owners_threshold);
		sock.recv_connected_value(out.reg_members_threshold);

		auto ownersCount = sock.recv_connected_primitive<member_count_t>();
		out.owners.resize(ownersCount);

		auto regMembersCount = sock.recv_connected_primitive<member_count_t>();
		out.reg_members.resize(regMembersCount);

		for (auto& owner : out.owners)
			sock.recv_connected_value(owner);

		for (auto& regMember : out.reg_members)
			sock.recv_connected_value(regMember);
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::MakeUserSetResponse& packet)
	{
		sock.send_connected_value(packet.user_set_id);
		send_pub_key(sock, packet.reg_layer_pub_key);
		send_pub_key(sock, packet.owner_layer_pub_key);
		send_priv_key_shard(sock, packet.reg_layer_priv_key_shard);
		send_priv_key_shard(sock, packet.owner_layer_priv_key_shard);
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::MakeUserSetResponse& out)
	{
		sock.recv_connected_value(out.user_set_id);
		recv_pub_key(sock, out.reg_layer_pub_key);
		recv_pub_key(sock, out.owner_layer_pub_key);
		recv_priv_key_shard(sock, out.reg_layer_priv_key_shard);
		recv_priv_key_shard(sock, out.owner_layer_priv_key_shard);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::GetUserSetsRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::GetUserSetsRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::GetUserSetsResponse& packet)
	{
		sock.send_connected_value(static_cast<userset_count_t>(packet.user_sets_ids.size()));
		for (const auto& userSetID : packet.user_sets_ids)
			sock.send_connected_value(userSetID);
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::GetUserSetsResponse& out)
	{
		auto usersetsCount = sock.recv_connected_primitive<userset_count_t>();
		out.user_sets_ids.resize(usersetsCount);
		for (auto& userSetID : out.user_sets_ids)
			sock.recv_connected_value(userSetID);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::GetMembersRequest& packet)
	{
		sock.send_connected_value(packet.user_set_id);
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::GetMembersRequest& out)
	{
		sock.recv_connected_value(out.user_set_id);
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::GetMembersResponse& packet)
	{
		sock.send_connected_value(static_cast<member_count_t>(packet.owners.size()));
		sock.send_connected_value(static_cast<member_count_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			sock.send_connected_value(owner);
		for (const auto& reg_member : packet.reg_members)
			sock.send_connected_value(reg_member);
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::GetMembersResponse& out)
	{
		auto ownersCount = sock.recv_connected_primitive<member_count_t>();
		out.owners.resize(ownersCount);
		auto regMembersCount = sock.recv_connected_primitive<member_count_t>();
		out.reg_members.resize(regMembersCount);
		for (auto& owner : out.owners)
			sock.recv_connected_value(owner);
		for (auto& regMember : out.reg_members)
			sock.recv_connected_value(regMember);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::DecryptRequest& packet)
	{
		sock.send_connected_value(packet.user_set_id);
		send_ciphertext(sock, packet.ciphertext);
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::DecryptRequest& out)
	{
		sock.recv_connected_value(out.user_set_id);
		recv_ciphertext(sock, out.ciphertext);
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::DecryptResponse& packet)
	{
		sock.send_connected_value(packet.op_id);
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::DecryptResponse& out)
	{
		sock.recv_connected_value(out.op_id);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::UpdateRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::UpdateRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::UpdateResponse& packet)
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

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::UpdateResponse& out)
	{
		// recv vector lengths
		auto addedAsOwnerCount = sock.recv_connected_primitive<userset_count_t>();
		auto addedAsRegMemberCount = sock.recv_connected_primitive<userset_count_t>();
		auto onLookupCount = sock.recv_connected_primitive<lookup_count_t>();
		auto toDecryptCount = sock.recv_connected_primitive<pending_count_t>();
		auto finishedDecryptionsCount = sock.recv_connected_primitive<res_count_t>();

		// recv added_as_owner records
		out.added_as_owner.resize(addedAsOwnerCount);
		for (auto& record : out.added_as_owner)
			recv_update_record(sock, record);

		// recv added_as_reg_member records
		out.added_as_reg_member.resize(addedAsRegMemberCount);
		for (auto& record : out.added_as_reg_member)
			recv_update_record(sock, record);

		// recv on_lookup records
		out.on_lookup.resize(onLookupCount);
		for (auto& record : out.on_lookup)
			sock.recv_connected_value(record);

		// recv to_decrypt records
		out.to_decrypt.resize(toDecryptCount);
		for (auto& record : out.to_decrypt)
			recv_update_record(sock, record);

		// recv finished_decryptions records
		out.finished_decryptions.resize(finishedDecryptionsCount);
		for (auto& record : out.finished_decryptions)
			recv_update_record(sock, record);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet)
	{
		sock.send_connected_value(packet.op_id);
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::DecryptParticipateRequest& out)
	{
		sock.recv_connected_value(out.op_id);
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::DecryptParticipateResponse& out)
	{
		sock.recv_connected_value(out.status);
	}

	void InlinePacketHandler::send_request_data(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet)
	{
		sock.send_connected_value(packet.op_id);
		send_decryption_part(sock, packet.decryption_part);
	}

	void InlinePacketHandler::recv_request_data(utils::Socket& sock, pkt::SendDecryptionPartRequest& out)
	{
		sock.recv_connected_value(out.op_id);
		recv_decryption_part(sock, out.decryption_part);
	}

	void InlinePacketHandler::send_response_data(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet)
	{
		(void)sock;
		(void)packet;
	}

	void InlinePacketHandler::recv_response_data(utils::Socket& sock, pkt::SendDecryptionPartResponse& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketHandler::send_pub_key(utils::Socket& sock, const PubKey& pubKey)
	{
		SockUtils::send_ecgroup_elem(sock, pubKey);
	}

	void InlinePacketHandler::recv_pub_key(utils::Socket& sock, PubKey& out)
	{
		SockUtils::recv_ecgroup_elem(sock, out);
	}

	void InlinePacketHandler::send_priv_key_shard_id(utils::Socket& sock, const PrivKeyShardID& shardID)
	{
		SockUtils::send_big_int(sock, shardID);
	}

	void InlinePacketHandler::recv_priv_key_shard_id(utils::Socket& sock, PrivKeyShardID& out)
	{
		SockUtils::recv_big_int(sock, out);
	}

	void InlinePacketHandler::send_priv_key_shard(utils::Socket& sock, const PrivKeyShard& shard)
	{
		send_priv_key_shard_id(sock, shard.first);
		SockUtils::send_big_int(sock, static_cast<const utils::BigInt&>(shard.second));
	}

	void InlinePacketHandler::recv_priv_key_shard(utils::Socket& sock, PrivKeyShard& out)
	{
		recv_priv_key_shard_id(sock, out.first);

		// converting second from BigInt
		utils::BigInt second;
		SockUtils::recv_big_int(sock, second);
		out.second = second;
	}

	void InlinePacketHandler::send_ciphertext(utils::Socket& sock, const Ciphertext& ciphertext)
	{
		const auto& [c1, c2, c3] = ciphertext;
		const auto& [c3a, c3b] = c3;

		SockUtils::send_ecgroup_elem(sock, c1);
		SockUtils::send_ecgroup_elem(sock, c2);
		
		sock.send_connected_value(static_cast<buffer_size_t>(c3a.size()));
		sock.send_connected_value(static_cast<buffer_size_t>(c3b.size()));
		sock.send_connected_value(c3);
	}

	void InlinePacketHandler::recv_ciphertext(utils::Socket& sock, Ciphertext& out)
	{
		auto& [c1, c2, c3] = out;
		auto& [c3a, c3b] = c3;

		SockUtils::recv_ecgroup_elem(sock, c1);
		SockUtils::recv_ecgroup_elem(sock, c2);

		// c3: reserve space then read directly from socket
		auto c3aSize = sock.recv_connected_primitive<buffer_size_t>();
		auto c3bSize = sock.recv_connected_primitive<buffer_size_t>();
		c3a.resize(c3aSize);
		sock.recv_connected_exact_into(c3a);
		c3b.resize(c3bSize);
		sock.recv_connected_exact_into(c3b);
	}

	void InlinePacketHandler::send_decryption_part(utils::Socket& sock, const DecryptionPart& part)
	{
		SockUtils::send_ecgroup_elem(sock, part);
	}

	void InlinePacketHandler::recv_decryption_part(utils::Socket& sock, DecryptionPart& out)
	{
		SockUtils::recv_ecgroup_elem(sock, out);
	}

	void InlinePacketHandler::send_update_record(utils::Socket& sock, const pkt::UpdateResponse::AddedAsOwnerRecord& record)
	{
		send_update_record(
			sock,
			reinterpret_cast<const pkt::UpdateResponse::AddedAsMemberRecord&>(record)
		);
		send_priv_key_shard(sock, record.owner_layer_priv_key_shard);
	}

	void InlinePacketHandler::recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsOwnerRecord& out)
	{
		recv_update_record(
			sock,
			reinterpret_cast<pkt::UpdateResponse::AddedAsMemberRecord&>(out)
		);
		recv_priv_key_shard(sock, out.owner_layer_priv_key_shard);
	}

	void InlinePacketHandler::send_update_record(utils::Socket& sock, const pkt::UpdateResponse::AddedAsMemberRecord& record)
	{
		sock.send_connected_value(record.user_set_id);
		send_pub_key(sock, record.reg_layer_pub_key);
		send_pub_key(sock, record.owner_layer_pub_key);
		send_priv_key_shard(sock, record.reg_layer_priv_key_shard);
	}

	void InlinePacketHandler::recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsMemberRecord& out)
	{
		sock.recv_connected_value(out.user_set_id);
		recv_pub_key(sock, out.reg_layer_pub_key);
		recv_pub_key(sock, out.owner_layer_pub_key);
		recv_priv_key_shard(sock, out.reg_layer_priv_key_shard);
	}

	void InlinePacketHandler::send_update_record(utils::Socket& sock, const pkt::UpdateResponse::ToDecryptRecord& record)
	{
		sock.send_connected_value(record.op_id);
		send_ciphertext(sock, record.ciphertext);
		sock.send_connected_value(static_cast<member_count_t>(record.shards_ids.size()));
		for (const auto& shardID : record.shards_ids)
			send_priv_key_shard_id(sock, shardID);
	}

	void InlinePacketHandler::recv_update_record(utils::Socket& sock, pkt::UpdateResponse::ToDecryptRecord& out)
	{
		sock.recv_connected_value(out.op_id);
		recv_ciphertext(sock, out.ciphertext);

		auto shardsIDsCount = sock.recv_connected_primitive<member_count_t>();
		out.shards_ids.resize(shardsIDsCount);
		for (auto& shardID : out.shards_ids)
			recv_priv_key_shard_id(sock, shardID);
	}

	void InlinePacketHandler::send_update_record(utils::Socket& sock, const pkt::UpdateResponse::FinishedDecryptionsRecord& record)
	{
		// NOTE: Assuming each shards IDs vector has is exactly one more than its corresponding parts vector
		sock.send_connected_value(static_cast<member_count_t>(record.reg_layer_parts.size()));
		sock.send_connected_value(static_cast<member_count_t>(record.owner_layer_parts.size()));
		sock.send_connected_value(record.op_id);
		for (const auto& part : record.reg_layer_parts)
			send_decryption_part(sock, part);
		for (const auto& part : record.owner_layer_parts)
			send_decryption_part(sock, part);
		for (const auto& shardID : record.reg_layer_shards_ids)
			send_priv_key_shard_id(sock, shardID);
		for (const auto& shardID : record.owner_layer_shards_ids)
			send_priv_key_shard_id(sock, shardID);
	}

	void InlinePacketHandler::recv_update_record(utils::Socket& sock, pkt::UpdateResponse::FinishedDecryptionsRecord& out)
	{
		// NOTE: Assuming each shards IDs vector has is exactly one more than its corresponding parts vector

		// recv sizes
		auto regLayerPartsCount = sock.recv_connected_primitive<member_count_t>();
		auto ownerLayerPartsCount = sock.recv_connected_primitive<member_count_t>();
		sock.recv_connected_value(out.op_id);

		// recv parts
		out.reg_layer_parts.resize(regLayerPartsCount);
		for (auto& part : out.reg_layer_parts)
			recv_decryption_part(sock, part);
		out.owner_layer_parts.resize(ownerLayerPartsCount);
		for (auto& part : out.owner_layer_parts)
			recv_decryption_part(sock, part);

		// recv shards IDs
		out.reg_layer_shards_ids.resize(regLayerPartsCount + 1);
		for (auto& shardID : out.reg_layer_shards_ids)
			recv_priv_key_shard_id(sock, shardID);
		out.owner_layer_shards_ids.resize(ownerLayerPartsCount + 1);
		for (auto& shardID : out.owner_layer_shards_ids)
			recv_priv_key_shard_id(sock, shardID);
	}
}

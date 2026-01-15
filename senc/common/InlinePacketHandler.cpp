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
	InlinePacketHandler::Self InlinePacketHandler::server(utils::Socket& sock)
	{
		Self res(sock);

		// receive & check protocol version
		auto protocolVersion = res._sock.recv_connected_primitive<std::uint8_t>();
		if (protocolVersion != pkt::PROTOCOL_VERSION)
		{
			res._sock.send_connected_primitive(false); // bad protocol version
			throw ConnEstablishException("Bad protocol version");
		}
		res._sock.send_connected_primitive(true); // protocol version OK

		return res;
	}

	InlinePacketHandler::Self InlinePacketHandler::client(utils::Socket& sock)
	{
		Self res(sock);

		// send protocol version
		res._sock.send_connected_primitive(pkt::PROTOCOL_VERSION);

		// receive flag indicating whether protocol version is OK
		const bool isProtocolVersoinOK = res._sock.recv_connected_primitive<bool>();
		if (!isProtocolVersoinOK)
			throw ConnEstablishException("Bad protocol version");

		return res;
	}

	void InlinePacketHandler::send_response_data(const pkt::ErrorResponse& packet)
	{
		_sock.send_connected_value(packet.msg);
	}

	void InlinePacketHandler::recv_response_data(pkt::ErrorResponse& out)
	{
		_sock.recv_connected_value(out.msg);
	}

	void InlinePacketHandler::send_request_data(const pkt::SignupRequest& packet)
	{
		_sock.send_connected_value(packet.username);
		_sock.send_connected_value(packet.password);
	}

	void InlinePacketHandler::recv_request_data(pkt::SignupRequest& out)
	{
		_sock.recv_connected_value(out.username);
		_sock.recv_connected_value(out.password);
	}

	void InlinePacketHandler::send_response_data(const pkt::SignupResponse& packet)
	{
		_sock.send_connected_value(packet.status);
	}

	void InlinePacketHandler::recv_response_data(pkt::SignupResponse& out)
	{
		_sock.recv_connected_value(out.status);
	}

	void InlinePacketHandler::send_request_data(const pkt::LoginRequest& packet)
	{
		_sock.send_connected_value(packet.username);
		_sock.send_connected_value(packet.password);
	}

	void InlinePacketHandler::recv_request_data(pkt::LoginRequest& out)
	{
		_sock.recv_connected_value(out.username);
		_sock.recv_connected_value(out.password);
	}

	void InlinePacketHandler::send_response_data(const pkt::LoginResponse& packet)
	{
		_sock.send_connected_value(packet.status);
	}

	void InlinePacketHandler::recv_response_data(pkt::LoginResponse& out)
	{
		_sock.recv_connected_value(out.status);
	}

	void InlinePacketHandler::send_request_data(const pkt::LogoutRequest& packet)
	{
		(void)packet;
	}

	void InlinePacketHandler::recv_request_data(pkt::LogoutRequest& out)
	{
		(void)out;
	}

	void InlinePacketHandler::send_response_data(const pkt::LogoutResponse& packet)
	{
		(void)packet;
	}

	void InlinePacketHandler::recv_response_data(pkt::LogoutResponse& out)
	{
		(void)out;
	}

	void InlinePacketHandler::send_request_data(const pkt::MakeUserSetRequest& packet)
	{
		_sock.send_connected_value(packet.owners_threshold);
		_sock.send_connected_value(packet.reg_members_threshold);
		_sock.send_connected_value(static_cast<member_count_t>(packet.owners.size()));
		_sock.send_connected_value(static_cast<member_count_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			_sock.send_connected_value(owner);
		for (const auto& regMember : packet.reg_members)
			_sock.send_connected_value(regMember);
	}

	void InlinePacketHandler::recv_request_data(pkt::MakeUserSetRequest& out)
	{
		_sock.recv_connected_value(out.owners_threshold);
		_sock.recv_connected_value(out.reg_members_threshold);

		auto ownersCount = _sock.recv_connected_primitive<member_count_t>();
		out.owners.resize(ownersCount);

		auto regMembersCount = _sock.recv_connected_primitive<member_count_t>();
		out.reg_members.resize(regMembersCount);

		for (auto& owner : out.owners)
			_sock.recv_connected_value(owner);

		for (auto& regMember : out.reg_members)
			_sock.recv_connected_value(regMember);
	}

	void InlinePacketHandler::send_response_data(const pkt::MakeUserSetResponse& packet)
	{
		_sock.send_connected_value(packet.user_set_id);
		send_pub_key(packet.reg_layer_pub_key);
		send_pub_key(packet.owner_layer_pub_key);
		send_priv_key_shard(packet.reg_layer_priv_key_shard);
		send_priv_key_shard(packet.owner_layer_priv_key_shard);
	}

	void InlinePacketHandler::recv_response_data(pkt::MakeUserSetResponse& out)
	{
		_sock.recv_connected_value(out.user_set_id);
		recv_pub_key(out.reg_layer_pub_key);
		recv_pub_key(out.owner_layer_pub_key);
		recv_priv_key_shard(out.reg_layer_priv_key_shard);
		recv_priv_key_shard(out.owner_layer_priv_key_shard);
	}

	void InlinePacketHandler::send_request_data(const pkt::GetUserSetsRequest& packet)
	{
		(void)packet;
	}

	void InlinePacketHandler::recv_request_data(pkt::GetUserSetsRequest& out)
	{
		(void)out;
	}

	void InlinePacketHandler::send_response_data(const pkt::GetUserSetsResponse& packet)
	{
		_sock.send_connected_value(static_cast<userset_count_t>(packet.user_sets_ids.size()));
		for (const auto& userSetID : packet.user_sets_ids)
			_sock.send_connected_value(userSetID);
	}

	void InlinePacketHandler::recv_response_data(pkt::GetUserSetsResponse& out)
	{
		auto usersetsCount = _sock.recv_connected_primitive<userset_count_t>();
		out.user_sets_ids.resize(usersetsCount);
		for (auto& userSetID : out.user_sets_ids)
			_sock.recv_connected_value(userSetID);
	}

	void InlinePacketHandler::send_request_data(const pkt::GetMembersRequest& packet)
	{
		_sock.send_connected_value(packet.user_set_id);
	}

	void InlinePacketHandler::recv_request_data(pkt::GetMembersRequest& out)
	{
		_sock.recv_connected_value(out.user_set_id);
	}

	void InlinePacketHandler::send_response_data(const pkt::GetMembersResponse& packet)
	{
		_sock.send_connected_value(static_cast<member_count_t>(packet.owners.size()));
		_sock.send_connected_value(static_cast<member_count_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			_sock.send_connected_value(owner);
		for (const auto& reg_member : packet.reg_members)
			_sock.send_connected_value(reg_member);
	}

	void InlinePacketHandler::recv_response_data(pkt::GetMembersResponse& out)
	{
		auto ownersCount = _sock.recv_connected_primitive<member_count_t>();
		out.owners.resize(ownersCount);
		auto regMembersCount = _sock.recv_connected_primitive<member_count_t>();
		out.reg_members.resize(regMembersCount);
		for (auto& owner : out.owners)
			_sock.recv_connected_value(owner);
		for (auto& regMember : out.reg_members)
			_sock.recv_connected_value(regMember);
	}

	void InlinePacketHandler::send_request_data(const pkt::DecryptRequest& packet)
	{
		_sock.send_connected_value(packet.user_set_id);
		send_ciphertext(packet.ciphertext);
	}

	void InlinePacketHandler::recv_request_data(pkt::DecryptRequest& out)
	{
		_sock.recv_connected_value(out.user_set_id);
		recv_ciphertext(out.ciphertext);
	}

	void InlinePacketHandler::send_response_data(const pkt::DecryptResponse& packet)
	{
		_sock.send_connected_value(packet.op_id);
	}

	void InlinePacketHandler::recv_response_data(pkt::DecryptResponse& out)
	{
		_sock.recv_connected_value(out.op_id);
	}

	void InlinePacketHandler::send_request_data(const pkt::UpdateRequest& packet)
	{
		(void)packet;
	}

	void InlinePacketHandler::recv_request_data(pkt::UpdateRequest& out)
	{
		(void)out;
	}

	void InlinePacketHandler::send_response_data(const pkt::UpdateResponse& packet)
	{
		// send vector lengths
		_sock.send_connected_value(static_cast<userset_count_t>(packet.added_as_owner.size()));
		_sock.send_connected_value(static_cast<userset_count_t>(packet.added_as_reg_member.size()));
		_sock.send_connected_value(static_cast<lookup_count_t>(packet.on_lookup.size()));
		_sock.send_connected_value(static_cast<pending_count_t>(packet.to_decrypt.size()));
		_sock.send_connected_value(static_cast<res_count_t>(packet.finished_decryptions.size()));

		// send added_as_owner records
		for (const auto& record : packet.added_as_owner)
			send_update_record(record);

		// send added_as_reg_member records
		for (const auto& record : packet.added_as_reg_member)
			send_update_record(record);
		
		// send on_lookup records
		for (const auto& record : packet.on_lookup)
			_sock.send_connected_value(record);

		// send to_decrypt records
		for (const auto& record : packet.to_decrypt)
			send_update_record(record);

		// send finished_decryptions records
		for (const auto& record : packet.finished_decryptions)
			send_update_record(record);
	}

	void InlinePacketHandler::recv_response_data(pkt::UpdateResponse& out)
	{
		// recv vector lengths
		auto addedAsOwnerCount = _sock.recv_connected_primitive<userset_count_t>();
		auto addedAsRegMemberCount = _sock.recv_connected_primitive<userset_count_t>();
		auto onLookupCount = _sock.recv_connected_primitive<lookup_count_t>();
		auto toDecryptCount = _sock.recv_connected_primitive<pending_count_t>();
		auto finishedDecryptionsCount = _sock.recv_connected_primitive<res_count_t>();

		// recv added_as_owner records
		out.added_as_owner.resize(addedAsOwnerCount);
		for (auto& record : out.added_as_owner)
			recv_update_record(record);

		// recv added_as_reg_member records
		out.added_as_reg_member.resize(addedAsRegMemberCount);
		for (auto& record : out.added_as_reg_member)
			recv_update_record(record);

		// recv on_lookup records
		out.on_lookup.resize(onLookupCount);
		for (auto& record : out.on_lookup)
			_sock.recv_connected_value(record);

		// recv to_decrypt records
		out.to_decrypt.resize(toDecryptCount);
		for (auto& record : out.to_decrypt)
			recv_update_record(record);

		// recv finished_decryptions records
		out.finished_decryptions.resize(finishedDecryptionsCount);
		for (auto& record : out.finished_decryptions)
			recv_update_record(record);
	}

	void InlinePacketHandler::send_request_data(const pkt::DecryptParticipateRequest& packet)
	{
		_sock.send_connected_value(packet.op_id);
	}

	void InlinePacketHandler::recv_request_data(pkt::DecryptParticipateRequest& out)
	{
		_sock.recv_connected_value(out.op_id);
	}

	void InlinePacketHandler::send_response_data(const pkt::DecryptParticipateResponse& packet)
	{
		_sock.send_connected_value(packet.status);
	}

	void InlinePacketHandler::recv_response_data(pkt::DecryptParticipateResponse& out)
	{
		_sock.recv_connected_value(out.status);
	}

	void InlinePacketHandler::send_request_data(const pkt::SendDecryptionPartRequest& packet)
	{
		_sock.send_connected_value(packet.op_id);
		send_decryption_part(packet.decryption_part);
	}

	void InlinePacketHandler::recv_request_data(pkt::SendDecryptionPartRequest& out)
	{
		_sock.recv_connected_value(out.op_id);
		recv_decryption_part(out.decryption_part);
	}

	void InlinePacketHandler::send_response_data(const pkt::SendDecryptionPartResponse& packet)
	{
		(void)packet;
	}

	void InlinePacketHandler::recv_response_data(pkt::SendDecryptionPartResponse& out)
	{
		(void)out;
	}

	InlinePacketHandler::InlinePacketHandler(utils::Socket& sock)
		: Base(sock) { }

	void InlinePacketHandler::send_pub_key(const PubKey& pubKey)
	{
		SockUtils::send_ecgroup_elem(_sock, pubKey);
	}

	void InlinePacketHandler::recv_pub_key(PubKey& out)
	{
		SockUtils::recv_ecgroup_elem(_sock, out);
	}

	void InlinePacketHandler::send_priv_key_shard_id(const PrivKeyShardID& shardID)
	{
		SockUtils::send_big_int(_sock, shardID);
	}

	void InlinePacketHandler::recv_priv_key_shard_id(PrivKeyShardID& out)
	{
		SockUtils::recv_big_int(_sock, out);
	}

	void InlinePacketHandler::send_priv_key_shard(const PrivKeyShard& shard)
	{
		send_priv_key_shard_id(shard.first);
		SockUtils::send_big_int(_sock, static_cast<const utils::BigInt&>(shard.second));
	}

	void InlinePacketHandler::recv_priv_key_shard(PrivKeyShard& out)
	{
		recv_priv_key_shard_id(out.first);

		// converting second from BigInt
		utils::BigInt second;
		SockUtils::recv_big_int(_sock, second);
		out.second = second;
	}

	void InlinePacketHandler::send_ciphertext(const Ciphertext& ciphertext)
	{
		const auto& [c1, c2, c3] = ciphertext;
		const auto& [c3a, c3b] = c3;

		SockUtils::send_ecgroup_elem(_sock, c1);
		SockUtils::send_ecgroup_elem(_sock, c2);
		
		_sock.send_connected_value(static_cast<buffer_size_t>(c3a.size()));
		_sock.send_connected_value(static_cast<buffer_size_t>(c3b.size()));
		_sock.send_connected_value(c3);
	}

	void InlinePacketHandler::recv_ciphertext(Ciphertext& out)
	{
		auto& [c1, c2, c3] = out;
		auto& [c3a, c3b] = c3;

		SockUtils::recv_ecgroup_elem(_sock, c1);
		SockUtils::recv_ecgroup_elem(_sock, c2);

		// c3: reserve space then read directly from socket
		auto c3aSize = _sock.recv_connected_primitive<buffer_size_t>();
		auto c3bSize = _sock.recv_connected_primitive<buffer_size_t>();
		c3a.resize(c3aSize);
		_sock.recv_connected_exact_into(c3a);
		c3b.resize(c3bSize);
		_sock.recv_connected_exact_into(c3b);
	}

	void InlinePacketHandler::send_decryption_part(const DecryptionPart& part)
	{
		SockUtils::send_ecgroup_elem(_sock, part);
	}

	void InlinePacketHandler::recv_decryption_part(DecryptionPart& out)
	{
		SockUtils::recv_ecgroup_elem(_sock, out);
	}

	void InlinePacketHandler::send_update_record(const pkt::UpdateResponse::AddedAsOwnerRecord& record)
	{
		send_update_record(
			reinterpret_cast<const pkt::UpdateResponse::AddedAsMemberRecord&>(record)
		);
		send_priv_key_shard(record.owner_layer_priv_key_shard);
	}

	void InlinePacketHandler::recv_update_record(pkt::UpdateResponse::AddedAsOwnerRecord& out)
	{
		recv_update_record(
			reinterpret_cast<pkt::UpdateResponse::AddedAsMemberRecord&>(out)
		);
		recv_priv_key_shard(out.owner_layer_priv_key_shard);
	}

	void InlinePacketHandler::send_update_record(const pkt::UpdateResponse::AddedAsMemberRecord& record)
	{
		_sock.send_connected_value(record.user_set_id);
		send_pub_key(record.reg_layer_pub_key);
		send_pub_key(record.owner_layer_pub_key);
		send_priv_key_shard(record.reg_layer_priv_key_shard);
	}

	void InlinePacketHandler::recv_update_record(pkt::UpdateResponse::AddedAsMemberRecord& out)
	{
		_sock.recv_connected_value(out.user_set_id);
		recv_pub_key(out.reg_layer_pub_key);
		recv_pub_key(out.owner_layer_pub_key);
		recv_priv_key_shard(out.reg_layer_priv_key_shard);
	}

	void InlinePacketHandler::send_update_record(const pkt::UpdateResponse::ToDecryptRecord& record)
	{
		_sock.send_connected_value(record.op_id);
		send_ciphertext(record.ciphertext);
		_sock.send_connected_value(static_cast<member_count_t>(record.shards_ids.size()));
		for (const auto& shardID : record.shards_ids)
			send_priv_key_shard_id(shardID);
	}

	void InlinePacketHandler::recv_update_record(pkt::UpdateResponse::ToDecryptRecord& out)
	{
		_sock.recv_connected_value(out.op_id);
		recv_ciphertext(out.ciphertext);

		auto shardsIDsCount = _sock.recv_connected_primitive<member_count_t>();
		out.shards_ids.resize(shardsIDsCount);
		for (auto& shardID : out.shards_ids)
			recv_priv_key_shard_id(shardID);
	}

	void InlinePacketHandler::send_update_record(const pkt::UpdateResponse::FinishedDecryptionsRecord& record)
	{
		// NOTE: Assuming each shards IDs vector has is exactly one more than its corresponding parts vector
		_sock.send_connected_value(static_cast<member_count_t>(record.reg_layer_parts.size()));
		_sock.send_connected_value(static_cast<member_count_t>(record.owner_layer_parts.size()));
		_sock.send_connected_value(record.op_id);
		for (const auto& part : record.reg_layer_parts)
			send_decryption_part(part);
		for (const auto& part : record.owner_layer_parts)
			send_decryption_part(part);
		for (const auto& shardID : record.reg_layer_shards_ids)
			send_priv_key_shard_id(shardID);
		for (const auto& shardID : record.owner_layer_shards_ids)
			send_priv_key_shard_id(shardID);
	}

	void InlinePacketHandler::recv_update_record(pkt::UpdateResponse::FinishedDecryptionsRecord& out)
	{
		// NOTE: Assuming each shards IDs vector has is exactly one more than its corresponding parts vector

		// recv sizes
		auto regLayerPartsCount = _sock.recv_connected_primitive<member_count_t>();
		auto ownerLayerPartsCount = _sock.recv_connected_primitive<member_count_t>();
		_sock.recv_connected_value(out.op_id);

		// recv parts
		out.reg_layer_parts.resize(regLayerPartsCount);
		for (auto& part : out.reg_layer_parts)
			recv_decryption_part(part);
		out.owner_layer_parts.resize(ownerLayerPartsCount);
		for (auto& part : out.owner_layer_parts)
			recv_decryption_part(part);

		// recv shards IDs
		out.reg_layer_shards_ids.resize(regLayerPartsCount + 1);
		for (auto& shardID : out.reg_layer_shards_ids)
			recv_priv_key_shard_id(shardID);
		out.owner_layer_shards_ids.resize(ownerLayerPartsCount + 1);
		for (auto& shardID : out.owner_layer_shards_ids)
			recv_priv_key_shard_id(shardID);
	}
}

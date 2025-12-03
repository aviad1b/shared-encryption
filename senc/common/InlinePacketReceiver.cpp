/*********************************************************************
 * \file   InlinePacketReceiver.cpp
 * \brief  Implementation of InlinePacketReceiver class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "InlinePacketReceiver.hpp"

#include "../utils/ranges.hpp"

namespace senc
{
	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out)
	{
		sock.recv_connected_value(out.msg);
	}

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::SignupRequest& out)
	{
		sock.recv_connected_value(out.username);
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::SignupResponse& out)
	{
		sock.recv_connected_value(out.status);
	}

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::LoginRequest& out)
	{
		sock.recv_connected_value(out.username);
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::LoginResponse& out)
	{
		sock.recv_connected_value(out.status);
	}

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::LogoutRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::LogoutResponse& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::MakeUserSetRequest& out)
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

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::MakeUserSetResponse& out)
	{
		sock.recv_connected_value(out.user_set_id);
		recv_pub_key(sock, out.pub_key1);
		recv_pub_key(sock, out.pub_key2);
		recv_priv_key_shard(sock, out.priv_key1_shard);
		recv_priv_key_shard(sock, out.priv_key2_shard);
	}

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::GetUserSetsRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::GetUserSetsResponse& out)
	{
		auto usersetsCount = sock.recv_connected_primitive<userset_count_t>();
		out.user_sets_ids.resize(usersetsCount);
		for (auto& userSetID : out.user_sets_ids)
			sock.recv_connected_value(userSetID);
	}

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::GetMembersRequest& out)
	{
		sock.recv_connected_value(out.user_set_id);
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::GetMembersResponse& out)
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

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::DecryptRequest& out)
	{
		sock.recv_connected_value(out.user_set_id);
		recv_ciphertext(sock, out.ciphertext);
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::DecryptResponse& out)
	{
		sock.recv_connected_value(out.op_id);
	}

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::UpdateRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::UpdateResponse& out)
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

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::DecryptParticipateRequest& out)
	{
		sock.recv_connected_value(out.op_id);
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::DecryptParticipateResponse& out)
	{
		sock.recv_connected_value(out.status);
	}

	void InlinePacketReceiver::recv_request_data(utils::Socket& sock, pkt::SendDecryptionPartRequest& out)
	{
		sock.recv_connected_value(out.op_id);
		recv_decryption_part(sock, out.decryption_part);
	}

	void InlinePacketReceiver::recv_response_data(utils::Socket& sock, pkt::SendDecryptionPartResponse& out)
	{
		(void)sock;
		(void)out;
	}

	bool InlinePacketReceiver::recv_big_int(utils::Socket& sock, utils::BigInt& out)
	{
		const bigint_size_t size = sock.recv_connected_primitive<bigint_size_t>();
		if (!size)
			return false; // nullopt recv'd
		utils::Buffer buff = sock.recv_connected_exact(size);
		out.Decode(buff.data(), buff.size());
		return true; // value recv'd
	}

	void InlinePacketReceiver::recv_ecgroup_elem(utils::Socket& sock, utils::ECGroup& out)
	{
		utils::BigInt x, y;

		// if x is sent as nullopt then elem is identity (and y isn't sent)
		if (!recv_big_int(sock, x))
		{
			out = utils::ECGroup::identity();
			return;
		}
		recv_big_int(sock, y);

		out = PubKey(std::move(x), std::move(y)); // TODO: Add c'tor with moved values to ECGroup
	}

	void InlinePacketReceiver::recv_pub_key(utils::Socket& sock, PubKey& out)
	{
		recv_ecgroup_elem(sock, out);
	}

	void InlinePacketReceiver::recv_priv_key_shard(utils::Socket& sock, PrivKeyShard& out)
	{
		sock.recv_connected_value(out.first);
		
		// converting second from BigInt
		utils::BigInt second;
		recv_big_int(sock, second);
		out.second = second;
	}

	void InlinePacketReceiver::recv_ciphertext(utils::Socket& sock, Ciphertext& out)
	{
		auto& [c1, c2, c3] = out;
		auto& [c3a, c3b] = c3;

		recv_ecgroup_elem(sock, c1);
		recv_ecgroup_elem(sock, c2);

		// c3: reserve space then read directly from socket
		auto c3aSize = sock.recv_connected_primitive<buffer_size_t>();
		auto c3bSize = sock.recv_connected_primitive<buffer_size_t>();
		c3a.resize(c3aSize);
		sock.recv_connected_exact_into(c3a);
		c3b.resize(c3bSize);
		sock.recv_connected_exact_into(c3b);
	}

	void InlinePacketReceiver::recv_decryption_part(utils::Socket& sock, DecryptionPart& out)
	{
		recv_ecgroup_elem(sock, out);
	}

	void InlinePacketReceiver::recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsOwnerRecord& out)
	{
		recv_update_record(
			sock,
			reinterpret_cast<pkt::UpdateResponse::AddedAsMemberRecord&>(out)
		);
		recv_priv_key_shard(sock, out.priv_key2_shard);
	}

	void InlinePacketReceiver::recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsMemberRecord& out)
	{
		sock.recv_connected_value(out.user_set_id);
		recv_pub_key(sock, out.pub_key1);
		recv_pub_key(sock, out.pub_key2);
		recv_priv_key_shard(sock, out.priv_key1_shard);
	}

	void InlinePacketReceiver::recv_update_record(utils::Socket& sock, pkt::UpdateResponse::ToDecryptRecord& out)
	{
		sock.recv_connected_value(out.op_id);
		recv_ciphertext(sock, out.ciphertext);
		
		auto shardsIDsCount = sock.recv_connected_primitive<member_count_t>();
		out.shards_ids.resize(shardsIDsCount);
		for (auto& shardID : out.shards_ids)
			sock.recv_connected_value(shardID);
	}

	void InlinePacketReceiver::recv_update_record(utils::Socket& sock, pkt::UpdateResponse::FinishedDecryptionsRecord& out)
	{
		// recv sizes
		auto parts1Count = sock.recv_connected_primitive<member_count_t>();
		auto parts2Count = sock.recv_connected_primitive<member_count_t>();
		sock.recv_connected_value(out.op_id);

		// recv parts
		out.parts1.resize(parts1Count);
		for (auto& part : out.parts1)
			recv_decryption_part(sock, part);
		out.parts2.resize(parts2Count);
		for (auto& part : out.parts2)
			recv_decryption_part(sock, part);
	}
}

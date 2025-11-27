/*********************************************************************
 * \file   PacketReceiver_impl.hpp
 * \brief  Implementation of PacketReceiver class.
 *
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "PacketReceiver.hpp"

namespace senc
{
	template <>
	inline pkt::ErrorResponse PacketReceiver::recv_response<pkt::ErrorResponse>(utils::Socket& sock)
	{
		pkt::ErrorResponse res{};
		sock.recv_connected_value(res.msg);
		return res;
	}

	template <>
	inline pkt::SignupRequest PacketReceiver::recv_request<pkt::SignupRequest>(utils::Socket& sock)
	{
		pkt::SignupRequest req{};
		sock.recv_connected_value(req.username);
		return req;
	}

	template <>
	inline pkt::SignupResponse PacketReceiver::recv_response<pkt::SignupResponse>(utils::Socket& sock)
	{
		pkt::SignupResponse res{};
		sock.recv_connected_value(res.status);
		return res;
	}

	template <>
	inline pkt::LoginRequest PacketReceiver::recv_request<pkt::LoginRequest>(utils::Socket& sock)
	{
		pkt::LoginRequest req{};
		sock.recv_connected_value(req.username);
		return req;
	}

	template <>
	inline pkt::LoginResponse PacketReceiver::recv_response<pkt::LoginResponse>(utils::Socket& sock)
	{
		pkt::LoginResponse res{};
		sock.recv_connected_value(res.status);
		return res;
	}

	template <>
	inline pkt::LogoutRequest PacketReceiver::recv_request<pkt::LogoutRequest>(utils::Socket& sock)
	{
		(void)sock;
		return pkt::LogoutRequest{};
	}

	template <>
	inline pkt::LogoutResponse PacketReceiver::recv_response<pkt::LogoutResponse>(utils::Socket& sock)
	{
		(void)sock;
		return pkt::LogoutResponse{};
	}

	template <>
	inline pkt::MakeUserSetRequest PacketReceiver::recv_request<pkt::MakeUserSetRequest>(utils::Socket& sock)
	{
		pkt::MakeUserSetRequest req{};

		sock.recv_connected_value(req.owners_threshold);
		sock.recv_connected_value(req.reg_members_threshold);

		auto ownersCount = sock.recv_connected_primitive<member_count_t>();
		req.owners.resize(ownersCount);

		auto regMembersCount = sock.recv_connected_primitive<member_count_t>();
		req.reg_members.resize(regMembersCount);

		for (auto& owner : req.owners)
			sock.recv_connected_value(owner);

		for (auto& regMember : req.reg_members)
			sock.recv_connected_value(regMember);

		return req;
	}

	template <>
	pkt::MakeUserSetResponse PacketReceiver::recv_response<pkt::MakeUserSetResponse>(utils::Socket& sock)
	{
		pkt::MakeUserSetResponse res{};
		sock.recv_connected_value(res.user_set_id);
		recv_pub_key(sock, res.pub_key1);
		recv_pub_key(sock, res.pub_key2);
		recv_priv_key_shard(sock, res.owner_priv_key1_shard);
		recv_priv_key_shard(sock, res.owner_priv_key2_shard);
		return res;
	}

	template <>
	pkt::GetUserSetsRequest PacketReceiver::recv_request<pkt::GetUserSetsRequest>(utils::Socket& sock)
	{
		(void)sock;
		return pkt::GetUserSetsRequest{};
	}

	template <>
	pkt::GetUserSetsResponse PacketReceiver::recv_response<pkt::GetUserSetsResponse>(utils::Socket& sock)
	{
		pkt::GetUserSetsResponse res{};
		auto usersetsCount = sock.recv_connected_primitive<userset_count_t>();
		res.user_sets_ids.resize(usersetsCount);
		for (auto& userSetID : res.user_sets_ids)
			sock.recv_connected_value(userSetID);
		return res;
	}

	template <>
	pkt::GetMembersRequest PacketReceiver::recv_request<pkt::GetMembersRequest>(utils::Socket& sock)
	{
		pkt::GetMembersRequest req{};
		sock.recv_connected_value(req.user_set_id);
		return req;
	}

	template <>
	pkt::GetMembersResponse PacketReceiver::recv_response<pkt::GetMembersResponse>(utils::Socket& sock)
	{
		pkt::GetMembersResponse res{};
		auto ownersCount = sock.recv_connected_primitive<member_count_t>();
		res.owners.resize(ownersCount);
		auto regMembersCount = sock.recv_connected_primitive<member_count_t>();
		res.reg_members.resize(regMembersCount);
		for (auto& owner : res.owners)
			sock.recv_connected_value(owner);
		for (auto& regMember : res.reg_members)
			sock.recv_connected_value(regMember);
		return res;
	}

	template <>
	pkt::DecryptRequest PacketReceiver::recv_request<pkt::DecryptRequest>(utils::Socket& sock)
	{
		pkt::DecryptRequest req{};
		sock.recv_connected_value(req.user_set_id);
		recv_ciphertext(sock, req.ciphertext);
		return req;
	}

	template <>
	pkt::DecryptResponse PacketReceiver::recv_response<pkt::DecryptResponse>(utils::Socket& sock)
	{
		pkt::DecryptResponse res{};
		sock.recv_connected_value(res.op_id);
		return res;
	}

	template <>
	pkt::UpdateRequest PacketReceiver::recv_request<pkt::UpdateRequest>(utils::Socket& sock)
	{
		(void)sock;
		return pkt::UpdateRequest{};
	}

	template <>
	pkt::UpdateResponse PacketReceiver::recv_response<pkt::UpdateResponse>(utils::Socket& sock)
	{
		pkt::UpdateResponse res{};

		// recv vector lengths
		auto addedAsOwnerCount = sock.recv_connected_primitive<userset_count_t>();
		auto addedAsRegMemberCount = sock.recv_connected_primitive<userset_count_t>();
		auto onLookupCount = sock.recv_connected_primitive<lookup_count_t>();
		auto toDecryptCount = sock.recv_connected_primitive<pending_count_t>();
		auto finishedDecryptionsCount = sock.recv_connected_primitive<res_count_t>();

		// recv added_as_owner records
		res.added_as_owner.resize(addedAsOwnerCount);
		for (auto& record : res.added_as_owner)
			recv_update_record(sock, record);

		// recv added_as_reg_member records
		res.added_as_reg_member.resize(addedAsRegMemberCount);
		for (auto& record : res.added_as_reg_member)
			recv_update_record(sock, record);

		// recv on_lookup records
		res.on_lookup.resize(onLookupCount);
		for (auto& record : res.on_lookup)
			sock.recv_connected_value(record);

		// recv to_decrypt records
		res.to_decrypt.resize(toDecryptCount);
		for (auto& record : res.to_decrypt)
			recv_update_record(sock, record);

		// recv finished_decryptions records
		res.finished_decryptions.resize(finishedDecryptionsCount);
		for (auto& record : res.finished_decryptions)
			recv_update_record(sock, record);

		return res;
	}
}

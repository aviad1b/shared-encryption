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
	}
}

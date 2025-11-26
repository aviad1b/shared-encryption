/*********************************************************************
 * \file   PacketSender.cpp
 * \brief  Implementation of PacketSender class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "PacketSender.hpp"

namespace senc
{
	void PacketSender::send_packet(utils::Socket& sock, const pkt::ErrorResponse& packet)
	{
		sock.send_connected_value(packet.msg);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::SignupRequest& packet)
	{
		sock.send_connected_value(packet.username);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::SignupResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::LoginRequest& packet)
	{
		sock.send_connected_value(packet.username);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::LoginResponse& packet)
	{
		sock.send_connected_value(packet.status);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::LogoutRequest& packet)
	{
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::LogoutResponse& packet)
	{
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::MakeUserSetRequest& packet)
	{
		sock.send_connected_value(packet.owners_threshold);
		sock.send_connected_value(packet.reg_members_threshold);
		sock.send_connected_value(static_cast<std::uint8_t>(packet.owners.size()));
		sock.send_connected_value(static_cast<std::uint8_t>(packet.reg_members.size()));
		for (const auto& reg_member : packet.reg_members)
			sock.send_connected_value(reg_member);
		for (const auto& owner : packet.owners)
			sock.send_connected_value(owner);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::MakeUserSetResponse& packet)
	{
		sock.send_connected_value(packet.user_set_id);
		send_pub_key(sock, packet.pub_key1);
		send_pub_key(sock, packet.pub_key2);
		send_priv_key_shard(sock, packet.owner_priv_key1_shard);
		send_priv_key_shard(sock, packet.owner_priv_key2_shard);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::GetUserSetsRequest& packet)
	{
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::GetUserSetsResponse& packet)
	{
		sock.send_connected_value(static_cast<std::uint8_t>(packet.user_sets_ids.size()));
		for (const auto& user_set_id : packet.user_sets_ids)
			sock.send_connected_value(user_set_id);
	}

	void PacketSender::send_big_int(utils::Socket& sock, const utils::BigInt& value)
	{
		sock.send_connected_value(static_cast<std::uint64_t>(value.MinEncodedSize()));

		utils::Buffer buff(value.MinEncodedSize());
		value.Encode(buff.data(), buff.size());
		sock.send_connected(buff);
	}

	void PacketSender::send_pub_key(utils::Socket& sock, const PubKey& pubKey)
	{
		send_big_int(sock, pubKey.x());
		send_big_int(sock, pubKey.y());
	}

	void PacketSender::send_priv_key_shard(utils::Socket& sock, const PrivKeyShard& shard)
	{
		sock.send_connected_value(shard.first);
		send_big_int(sock, static_cast<const utils::BigInt&>(shard.second));
	}
}

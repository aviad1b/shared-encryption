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
		for (const auto& owner : packet.owners)
			sock.send_connected_value(owner);
		for (const auto& reg_member : packet.reg_members)
			sock.send_connected_value(reg_member);
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

	void PacketSender::send_packet(utils::Socket& sock, const pkt::GetMembersRequest& packet)
	{
		sock.send_connected_value(packet.user_set_id);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::GetMembersResponse& packet)
	{
		sock.send_connected_value(static_cast<std::uint8_t>(packet.owners.size()));
		sock.send_connected_value(static_cast<std::uint8_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			sock.send_connected_value(owner);
		for (const auto& reg_member : packet.reg_members)
			sock.send_connected_value(reg_member);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::DecryptRequest& packet)
	{
		sock.send_connected_value(packet.user_set_id);
		send_ciphertext(sock, packet.ciphertext);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::DecryptResponse& packet)
	{
		sock.send_connected_value(packet.op_id);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::UpdateRequest& packet)
	{
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::UpdateResponse& packet)
	{
		// send vector lengths
		sock.send_connected_value(static_cast<std::uint8_t>(packet.added_as_owner.size()));
		sock.send_connected_value(static_cast<std::uint8_t>(packet.added_as_reg_member.size()));
		sock.send_connected_value(static_cast<std::uint8_t>(packet.on_lookup.size()));
		sock.send_connected_value(static_cast<std::uint8_t>(packet.to_decrypt.size()));
		sock.send_connected_value(static_cast<std::uint8_t>(packet.finished_decryptions.size()));

		// send added_as_owner records
		for (const auto& record : packet.added_as_owner)
		{
			sock.send_connected_value(record.user_set_id);
			send_pub_key(sock, record.pub_key1);
			send_pub_key(sock, record.pub_key2);
			send_priv_key_shard(sock, record.priv_key1_shard);
			send_priv_key_shard(sock, record.priv_key2_shard);
		}

		// send added_as_reg_member records
		for (const auto& record : packet.added_as_reg_member)
		{
			sock.send_connected_value(record.user_set_id);
			send_pub_key(sock, record.pub_key1);
			send_pub_key(sock, record.pub_key2);
			send_priv_key_shard(sock, record.priv_key1_shard);
		}
		
		// send on_lookup records
		for (const auto& record : packet.on_lookup)
			sock.send_connected_value(record);

		// send to_decrypt records
		for (const auto& record : packet.to_decrypt)
		{
			sock.send_connected_value(record.op_id);
			send_ciphertext(sock, record.ciphertext);
			sock.send_connected_value(static_cast<std::uint32_t>(record.shards_ids.size()));
			for (const auto& shardID : record.shards_ids)
				sock.send_connected_value(shardID);
		}

		// send finished_decryptions records
		for (const auto& record : packet.finished_decryptions)
		{
			// send parts counts
			sock.send_connected_value(static_cast<std::uint32_t>(record.parts1.size()));
			sock.send_connected_value(static_cast<std::uint32_t>(record.parts2.size()));
			sock.send_connected_value(record.op_id);
			sock.send_connected_value(record.user_set_id);
			for (const auto& part : record.parts1)
				send_decryption_part(sock, part);
			for (const auto& part : record.parts2)
				send_decryption_part(sock, part);
		}
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet)
	{
		sock.send_connected_value(packet.op_id);
	}

	void PacketSender::send_packet(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet)
	{
		sock.send_connected_value(packet.status);
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

	void PacketSender::send_ciphertext(utils::Socket& sock, const Ciphertext& ciphertext)
	{
		const auto& [c1, c2, c3] = ciphertext;
		const auto& [c3a, c3b] = c3;

		// send size dividers
		std::initializer_list<std::uint64_t> sizes = {
			static_cast<std::uint64_t>(c1.x().MinEncodedSize()),
			static_cast<std::uint64_t>(c1.y().MinEncodedSize()),
			static_cast<std::uint64_t>(c2.x().MinEncodedSize()),
			static_cast<std::uint64_t>(c2.y().MinEncodedSize()),
			static_cast<std::uint64_t>(c3a.size()),
			static_cast<std::uint64_t>(c3b.size())
		};
		for (std::uint64_t size : sizes)
			sock.send_connected_value(size);

		// send actual data
		utils::Buffer buff(std::max(sizes));

		c1.x().Encode(buff.data(), c1.x().MinEncodedSize());
		sock.send_connected(buff.data(), c1.x().MinEncodedSize());
		c1.y().Encode(buff.data(), c1.y().MinEncodedSize());
		sock.send_connected(buff.data(), c1.y().MinEncodedSize());

		c2.x().Encode(buff.data(), c2.x().MinEncodedSize());
		sock.send_connected(buff.data(), c2.x().MinEncodedSize());
		c2.y().Encode(buff.data(), c2.y().MinEncodedSize());
		sock.send_connected(buff.data(), c2.y().MinEncodedSize());

		sock.send_connected_value(c3);
	}

	void PacketSender::send_decryption_part(utils::Socket& sock, const DecryptionPart& part)
	{
		send_big_int(sock, part.x());
		send_big_int(sock, part.y());
	}
}

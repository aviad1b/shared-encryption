/*********************************************************************
 * \file   PacketReceiver.hpp
 * \brief  Header of PacketSender class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../utils/Socket.hpp"
#include "packets.hpp"

namespace senc
{
	class PacketReceiver
	{
	public:
		PacketReceiver() = delete;

		template <pkt::Request T>
		static inline T recv_request(utils::Socket& sock) { static_assert(false, "Unhandled request detected"); }

		template <pkt::Response T>
		static inline T recv_response(utils::Socket& sock) { static_assert(false, "Unhandled response detected"); }

	private:
		static void recv_big_int(utils::Socket& sock, utils::BigInt& out);

		static void recv_pub_key(utils::Socket& sock, PubKey& out);
		static void recv_priv_key_shard(utils::Socket& sock, PrivKeyShard& out);

		static void recv_ciphertext(utils::Socket& sock, Ciphertext& out);
		static void recv_decryption_part(utils::Socket& sock, DecryptionPart& out);

		static void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsOwnerRecord& out);
		static void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::AddedAsMemberRecord& out);
		static void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::ToDecryptRecord& out);
		static void recv_update_record(utils::Socket& sock, pkt::UpdateResponse::FinishedDecryptionsRecord& out);
	};
}

#include "PacketReceiver_impl.hpp"

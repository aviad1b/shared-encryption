/*********************************************************************
 * \file   PacketReceiver.cpp
 * \brief  Non-template implementation of PacketReceiver class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "PacketReceiver.hpp"

namespace senc
{
	void PacketReceiver::recv_big_int(utils::Socket& sock, utils::BigInt& out)
	{
		const bigint_size_t size = sock.recv_connected_primitive<bigint_size_t>();
		utils::Buffer buff = sock.recv_connected_exact(size);
		out.Decode(buff.data(), buff.size());
	}

	void PacketReceiver::recv_pub_key(utils::Socket& sock, PubKey& out)
	{
		utils::BigInt x, y;
		recv_big_int(sock, x);
		recv_big_int(sock, y);

		out = PubKey(std::move(x), std::move(y)); // TODO: Add c'tor with moved values to ECGroup
	}

	void PacketReceiver::recv_priv_key_shard(utils::Socket& sock, PrivKeyShard& out)
	{
		sock.recv_connected_value(out.first);
		
		// converting second from BigInt
		utils::BigInt second;
		recv_big_int(sock, second);
		out.second = second;
	}
}

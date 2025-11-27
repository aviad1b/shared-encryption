/*********************************************************************
 * \file   PacketReceiver.cpp
 * \brief  Non-template implementation of PacketReceiver class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "PacketReceiver.hpp"

#include "../utils/ranges.hpp"

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

	void PacketReceiver::recv_ciphertext(utils::Socket& sock, Ciphertext& out)
	{
		auto& [c1, c2, c3] = out;
		auto& [c3a, c3b] = c3;

		// recv size dividers
		std::array<utils::BigInt, 4> bigintValues{};
		std::array<bigint_size_t, 4> bigintSizes{};
		buffer_size_t c3aSize = 0, c3bSize = 0;
		for (auto& size : bigintSizes)
			sock.recv_connected_value(size);
		sock.recv_connected_value(c3aSize);
		sock.recv_connected_value(c3bSize);

		// recv actual data:

		// bigints: use a one-size-fits-each buffer
		utils::Buffer buff(*std::max_element(bigintSizes.begin(), bigintSizes.end()));
		for (auto& [bigintValue, bigintSize] : utils::views::zip(bigintValues, bigintSizes))
		{
			sock.recv_connected_exact_into(buff.data(), bigintSize);
			bigintValue.Encode(buff.data(), bigintSize);
		}

		// c3: reserve space then read directly from socket
		c3a.resize(c3aSize);
		sock.recv_connected_exact_into(c3a);
		c3b.resize(c3bSize);
		sock.recv_connected_exact_into(c3b);
	}
}

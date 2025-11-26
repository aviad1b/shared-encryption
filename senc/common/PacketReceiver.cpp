#include "PacketReceiver.hpp"
/*********************************************************************
 * \file   PacketReceiver.cpp
 * \brief  Non-template implementation of PacketReceiver class.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

namespace senc
{
	void PacketReceiver::recv_big_int(utils::Socket& sock, utils::BigInt& out)
	{
		const bigint_size_t size = sock.recv_connected_primitive<bigint_size_t>();
		utils::Buffer buff = sock.recv_connected_exact(size);
		out.Decode(buff.data(), buff.size());
	}
}

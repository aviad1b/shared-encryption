/*********************************************************************
 * \file   SockUtils.cpp
 * \brief  Implementation of `SockUtils` static class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "SockUtils.hpp"

namespace senc
{
	void SockUtils::send_big_int(utils::Socket& sock, const std::optional<utils::BigInt>& value)
	{
		if (!value.has_value())
		{
			sock.send_connected_value(static_cast<utils::bigint_size_t>(0));
			return;
		}

		sock.send_connected_value(static_cast<utils::bigint_size_t>(value->MinEncodedSize()));

		utils::Buffer buff(value->MinEncodedSize());
		value->Encode(buff.data(), buff.size());
		sock.send_connected(buff);
	}

	bool SockUtils::recv_big_int(utils::Socket& sock, utils::BigInt& out)
	{
		const utils::bigint_size_t size = sock.recv_connected_primitive<utils::bigint_size_t>();
		if (!size)
			return false; // nullopt recv'd
		utils::Buffer buff = sock.recv_connected_exact(size);
		out.Decode(buff.data(), buff.size());
		return true; // value recv'd
	}

	void SockUtils::send_ecgroup_elem(utils::Socket& sock, const utils::ECGroup& elem)
	{
		// if x is sent as nullopt then elem is identity (and y isn't sent)
		if (elem.is_identity())
		{
			send_big_int(sock, std::nullopt);
			return;
		}
		send_big_int(sock, elem.x());
		send_big_int(sock, elem.y());
	}

	void SockUtils::recv_ecgroup_elem(utils::Socket& sock, utils::ECGroup& out)
	{
		utils::BigInt x, y;

		// if x is sent as nullopt then elem is identity (and y isn't sent)
		if (!recv_big_int(sock, x))
		{
			out = utils::ECGroup::identity();
			return;
		}
		recv_big_int(sock, y);

		out = utils::ECGroup(std::move(x), std::move(y));
	}
}

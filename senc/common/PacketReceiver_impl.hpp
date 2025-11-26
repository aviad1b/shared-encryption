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
}

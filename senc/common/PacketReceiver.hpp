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
	};
}

#include "PacketReceiver_impl.hpp"

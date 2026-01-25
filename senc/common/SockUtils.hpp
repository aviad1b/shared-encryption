/*********************************************************************
 * \file   SockUtils.hpp
 * \brief  Header of `SockUtils` static class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/ECGroup.hpp"
#include "../utils/Socket.hpp"

namespace senc
{
	class SockUtils
	{
	public:
		SockUtils() = delete;

		/**
		 * @brief Sends a big integer through a socket.
		 * @param sock Socket to send through.
		 * @param value Big integer value to send.
		 */
		static void send_big_int(utils::Socket& sock, const std::optional<utils::BigInt>& value);

		/**
		 * @brief Receives a big integer through a socket.
		 * @param sock Socket to receive through.
		 * @param out Variable to store received data to.
		 * @return `true` if received bit integer, otherwise `false` (got "null").
		 */
		static bool recv_big_int(utils::Socket& sock, utils::BigInt& out);

		/**
		 * @brief Sends an Elliptic Curve element through a socket.
		 * @param sock Socket to send through.
		 * @param elem Elliptic Curve group element to send.
		 */
		static void send_ecgroup_elem(utils::Socket& sock, const utils::ECGroup& elem);

		/**
		 * @brief Receives an Elliptic Curve element through a socket.
		 * @param sock Socket to receive through.
		 * @param out Variable to store received data to.
		 */
		static void recv_ecgroup_elem(utils::Socket& sock, utils::ECGroup& out);
	};
}

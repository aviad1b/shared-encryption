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

	template <>
	inline pkt::SignupRequest PacketReceiver::recv_request<pkt::SignupRequest>(utils::Socket& sock)
	{
		pkt::SignupRequest req{};
		sock.recv_connected_value(req.username);
		return req;
	}

	template <>
	inline pkt::SignupResponse PacketReceiver::recv_response<pkt::SignupResponse>(utils::Socket& sock)
	{
		pkt::SignupResponse res{};
		sock.recv_connected_value(res.status);
		return res;
	}

	template <>
	inline pkt::LoginRequest PacketReceiver::recv_request<pkt::LoginRequest>(utils::Socket& sock)
	{
		pkt::LoginRequest req{};
		sock.recv_connected_value(req.username);
		return req;
	}

	template <>
	inline pkt::LoginResponse PacketReceiver::recv_response<pkt::LoginResponse>(utils::Socket& sock)
	{
		pkt::LoginResponse res{};
		sock.recv_connected_value(res.status);
		return res;
	}
}

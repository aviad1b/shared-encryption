/*********************************************************************
 * \file   tests_utils.hpp
 * \brief  Header of utilities for tests.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <gtest/gtest.h>
#include <algorithm>
#include <future>
#include <thread>
#include <tuple>
#include "../server/Server.hpp"
#include "../utils/concepts.hpp"
#include "../utils/Random.hpp"
#include "../utils/Socket.hpp"
#include "../utils/hash.hpp"

constexpr std::size_t CONN_RETRY_COUNT = 10;

/**
 * @brief Prepares local TCP connection for test.
 */
template <senc::utils::IPType IP = senc::utils::IPv4>
std::tuple<senc::utils::TcpSocket<IP>, senc::utils::TcpSocket<IP>> prepare_tcp()
{
	using senc::utils::TcpSocket;
	using senc::utils::Random;
	using senc::utils::Port;

	TcpSocket<IP> listenSock, sendSock;
	std::promise<TcpSocket<IP>> p;
	std::future<TcpSocket<IP>> f = p.get_future();

	// try selecting port `CONN_RETRY_COUNT-1` times
	std::optional<Port> port;
	for (std::size_t i = 1; i < CONN_RETRY_COUNT && !port.has_value(); ++i)
	{
		port = Random<Port>::sample_from_range(49152, 65535);

		try { listenSock.bind(*port); }
		catch (const senc::utils::SocketException&) { port.reset(); }
	}
	// if still failed, try another time, this time without a `try` block
	if (!port.has_value())
	{
		port = Random<Port>::sample_from_range(49152, 65535);
		listenSock.bind(*port);
	}
	listenSock.listen();

	std::jthread t(
		[&listenSock, &p]()
		{
			try { p.set_value(listenSock.accept().first); }
			catch (...) { p.set_exception(std::current_exception()); }
		}
	);

	sendSock.connect(IP::loopback(), *port);

	TcpSocket<IP> recvSock = f.get();

	return { std::move(sendSock), std::move(recvSock) };
}

/**
 * @brief Makes new server, given all args except for port.
 */
template <senc::utils::IPType IP>
std::unique_ptr<senc::server::IServer> new_server(auto&&... args)
{
	// try selecting port `CONN_RETRY_COUNT-1` times
	for (std::size_t i = 1; i < CONN_RETRY_COUNT; ++i)
	{
		auto port = Random<Port>::sample_from_range(49152, 65535);
		try { return std::make_unique<senc::server::Server<IP>>(port, args...); }
		catch (const senc::utils::SocketException&) { }
	}
	// if still failed, try another time, this time without a `try` block
	auto port = Random<Port>::sample_from_range(49152, 65535);
	return std::make_unique<senc::server::Server<IP>>(port, args...);
}

/**
 * @brief Checks if two vectors have the same elements, regardless of order.
 */
template <typename T>
void EXPECT_SAME_ELEMS(const std::vector<T>& a, const std::vector<T>& b)
{
	senc::utils::HashSet<T> setA(a.begin(), a.end());
	senc::utils::HashSet<T> setB(b.begin(), b.end());
	EXPECT_EQ(setA, setB);
}

template <typename C, typename T>
void EXPECT_CONTAINS(const C& container, const T& elem)
{
	if constexpr (senc::utils::HasContainsMethod<C, T>)
		EXPECT_TRUE(container.contains(elem));
	else
		EXPECT_NE(std::find(container.begin(), container.end(), elem), container.end());
}

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
#include "../utils/concepts.hpp"
#include "../utils/Random.hpp"
#include "../utils/Socket.hpp"
#include "../utils/hash.hpp"

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

	auto port = Random<Port>::sample_from_range(49152, 65535);

	listenSock.bind(port);
	listenSock.listen();

	std::jthread t(
		[&listenSock, &p]()
		{
			try { p.set_value(listenSock.accept().first); }
			catch (...) { p.set_exception(std::current_exception()); }
		}
	);

	sendSock.connect(IP::loopback(), port);

	TcpSocket<IP> recvSock = f.get();

	return { std::move(sendSock), std::move(recvSock) };
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

/*********************************************************************
 * \file   tests_utils.cpp
 * \brief  Implementation of utilities for tests.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "tests_utils.hpp"

#include <thread>
#include <future>

using senc::utils::TcpSocket;
using senc::utils::IPv4;

std::tuple<TcpSocket<IPv4>, TcpSocket<IPv4>> prepare_tcp()
{
	TcpSocket<IPv4> listenSock, sendSock;
	std::promise<TcpSocket<IPv4>> p;
	std::future<TcpSocket<IPv4>> f = p.get_future();

	listenSock.bind(4350);
	listenSock.listen();

	std::jthread t(
		[&listenSock, &p]()
		{
			try { p.set_value(listenSock.accept()); }
			catch (...) { p.set_exception(std::current_exception()); }
		}
	);

	sendSock.connect("127.0.0.1", 4350);

	TcpSocket<IPv4> recvSock = f.get();

	return { std::move(sendSock), std::move(recvSock) };
}

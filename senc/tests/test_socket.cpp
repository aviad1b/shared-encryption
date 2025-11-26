/*********************************************************************
 * \file   test_socket.cpp
 * \brief  File containing tests of the sockets utils (`Sockets.hpp`).
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <thread>
#include <future>
#include "../utils/Socket.hpp"

using senc::utils::UdpSocket;
using senc::utils::TcpSocket;
using senc::utils::Buffer;
using senc::utils::byte;
using senc::utils::IPv4;
using senc::utils::IPv6;

/**
 * @brief Tests basic IPv4 construction.
 */
TEST(SocketTests, TestIPv4)
{
	IPv4 ip("1.2.3.4");
	EXPECT_EQ(ip.as_str(), "1.2.3.4");
}

/**
 * @brief Tests basic IPv6 construction.
 */
TEST(SocketTests, TestIPv6)
{
	IPv6 ip("fd30:cb0a:c87a:0157:a1b2:c3d4:e5f6:7890");
	EXPECT_EQ(ip.as_str(), "fd30:cb0a:c87a:0157:a1b2:c3d4:e5f6:7890");
}

/**
 * @brief Tests basic UDP send and recieve.
 */
TEST(SocketTests, TestUDP)
{
	const Buffer sendData { byte(1), byte(2), byte(3) };
	UdpSocket<IPv4> sock1, sock2;

	sock1.bind(4350);

	sock2.send_to(sendData, "127.0.0.1", 4350);

	auto recvData = sock1.recv_from(sendData.size()).data;
	EXPECT_EQ(sendData, recvData);
}

/**
 * @brief Prepares local TCP connection for test.
 */
static std::tuple<TcpSocket<IPv4>, TcpSocket<IPv4>> prepare_tcp()
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

/**
 * @brief Tests basic TCP send and recieve.
 */
TEST(SocketTests, TestTCP)
{
	auto [sendSock, recvSock] = prepare_tcp();
	
	const Buffer sendData { byte(1), byte(2), byte(3) };

	sendSock.send_connected(sendData);
	auto recvData = recvSock.recv_connected(sendData.size());
	EXPECT_EQ(sendData, recvData);
}

/**
 * @brief Tests send and recv of strings over TCP.
 */
TEST(SocketTests, TestStrTCP)
{
	auto [sendSock, recvSock] = prepare_tcp();

	const std::string sendStr = "abcd";
	const Buffer sendBytes = { 1, 2, 3 };
	sendSock.send_connected_str(sendStr);
	sendSock.send_connected(sendBytes);

	// recieve string with three chars at a time, causing the beggining of next input to be leftover
	auto recv_str = recvSock.recv_connected_str<std::string, 3>();

	auto recv_bytes = recvSock.recv_connected(3);

	EXPECT_EQ(sendStr, recv_str);
	EXPECT_EQ(sendBytes, recv_bytes);

	const std::wstring sendWStr = L"abcd";
	sendSock.send_connected_str(sendWStr);
	sendSock.send_connected(sendBytes);

	// recieve string with three chars at a time, causing the beggining of next input to be leftover
	auto recv_wstr = recvSock.recv_connected_str<std::wstring, 3>();

	recv_bytes = recvSock.recv_connected(3);

	EXPECT_EQ(sendWStr, recv_wstr);
	EXPECT_EQ(sendBytes, recv_bytes);
}

TEST(SocketTests, TestExactTCP)
{
	auto [sendSock, recvSock] = prepare_tcp();

	const Buffer five = { 1, 2, 3, 4, 5 };
	const Buffer four = { 1, 2, 3, 4 };
	const Buffer last = { 5 };
	sendSock.send_connected(five);
	auto recv_four = recvSock.recv_connected_exact(4);
	EXPECT_EQ(recv_four, four);
	auto recv_last = recvSock.recv_connected(100);
	EXPECT_EQ(recv_last, last);
}

TEST(SocketTests, TestTupleTCP)
{
	auto [sendSock, recvSock] = prepare_tcp();
	auto sendTpl = std::make_tuple(Buffer{1, 2, 3}, std::string("hello"), 5);
	auto recvTpl = std::make_tuple(Buffer(3, byte(0)), std::string(), 0);

	sendSock.send_connected_value(sendTpl);
	recvSock.recv_connected_value(recvTpl);

	EXPECT_EQ(sendTpl, recvTpl);
}

/*********************************************************************
 * \file   test_socket.cpp
 * \brief  File containing tests of the sockets utils (`Sockets.hpp`).
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include "../utils/Socket.hpp"
#include "tests_utils.hpp"

using senc::utils::UdpSocket;
using senc::utils::TcpSocket;
using senc::utils::Buffer;
using senc::utils::byte;
using senc::utils::IPv4;
using senc::utils::IPv6;

/**
 * @brief Tests basic IPv4 construction.
 */
TEST(SocketTests, IPv4ConstructsFromString)
{
	IPv4 ip("1.2.3.4");
	EXPECT_EQ(ip.as_str(), "1.2.3.4");
}

/**
 * @brief Tests basic IPv6 construction.
 */
TEST(SocketTests, IPv6ConstructsFromString)
{
	IPv6 ip("fd30:cb0a:c87a:0157:a1b2:c3d4:e5f6:7890");
	EXPECT_EQ(ip.as_str(), "fd30:cb0a:c87a:0157:a1b2:c3d4:e5f6:7890");
}

/**
 * @brief Tests basic UDP send and recieve.
 */
TEST(SocketTests, UdpSendsAndReceivesData)
{
	const Buffer sendData { byte(1), byte(2), byte(3) };
	UdpSocket<IPv4> sock1, sock2;

	sock1.bind(4350);

	sock2.send_to(sendData, "127.0.0.1", 4350);

	auto recvData = sock1.recv_from(sendData.size()).data;
	EXPECT_EQ(sendData, recvData);
}

/**
 * @brief Tests basic TCP send and recieve.
 */
TEST(SocketTests, TcpSendsAndReceivesData)
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
TEST(SocketTests, TcpSendsAndReceivesStrings)
{
	auto [sendSock, recvSock] = prepare_tcp();

	const std::string sendStr = "abcd";
	const Buffer sendBytes = { 1, 2, 3 };
	sendSock.send_connected_str(sendStr);
	sendSock.send_connected(sendBytes);

	// recieve string with three chars at a time, causing the beggining of next input to be leftover
	auto recvStr = recvSock.recv_connected_str<std::string, 3>();

	auto recvBytes = recvSock.recv_connected(3);

	EXPECT_EQ(sendStr, recvStr);
	EXPECT_EQ(sendBytes, recvBytes);

	const std::wstring sendWStr = L"abcd";
	sendSock.send_connected_str(sendWStr);
	sendSock.send_connected(sendBytes);

	// recieve string with three chars at a time, causing the beggining of next input to be leftover
	auto recvWStr = recvSock.recv_connected_str<std::wstring, 3>();

	recvBytes = recvSock.recv_connected(3);

	EXPECT_EQ(sendWStr, recvWStr);
	EXPECT_EQ(sendBytes, recvBytes);
}

TEST(SocketTests, TcpReceivesExactByteCount)
{
	auto [sendSock, recvSock] = prepare_tcp();

	const Buffer five = { 1, 2, 3, 4, 5 };
	const Buffer four = { 1, 2, 3, 4 };
	const Buffer last = { 5 };
	sendSock.send_connected(five);
	auto recvFour = recvSock.recv_connected_exact(4);
	EXPECT_EQ(recvFour, four);
	auto recvLast = recvSock.recv_connected(100);
	EXPECT_EQ(recvLast, last);
}

TEST(SocketTests, TcpSerializesAndDeserializesTuples)
{
	auto [sendSock, recvSock] = prepare_tcp();
	auto sendTpl = std::make_tuple(Buffer{1, 2, 3}, std::string("hello"), 5);
	auto recvTpl = std::make_tuple(Buffer(3, byte(0)), std::string(), 0);

	sendSock.send_connected_value(sendTpl);
	recvSock.recv_connected_value(recvTpl);

	EXPECT_EQ(sendTpl, recvTpl);
}

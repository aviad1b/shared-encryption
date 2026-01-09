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
using senc::utils::IPType;
using senc::utils::Buffer;
using senc::utils::byte;
using senc::utils::IPv4;
using senc::utils::IPv6;

/**
 * @brief Tests basic IPv4 construction.
 */
TEST(IPTests, IPv4ConstructsFromString)
{
	IPv4 ip("1.2.3.4");
	EXPECT_EQ(ip.as_str(), "1.2.3.4");
}

/**
 * @brief Tests basic IPv6 construction.
 */
TEST(IPTests, IPv6ConstructsFromString)
{
	IPv6 ip("fd30:cb0a:c87a:0157:a1b2:c3d4:e5f6:7890");
	EXPECT_EQ(ip.as_str(), "fd30:cb0a:c87a:0157:a1b2:c3d4:e5f6:7890");
}

template <IPType IP_>
struct SocketTests : public testing::Test { };

using IPTypes = testing::Types<IPv4, IPv6>;
TYPED_TEST_SUITE(SocketTests, IPTypes);

/**
 * @brief Tests basic UDP send and recieve.
 */
TYPED_TEST(SocketTests, UdpSendsAndReceivesData)
{
	using IP = TypeParam;

	const Buffer sendData { byte(1), byte(2), byte(3) };
	UdpSocket<IP> sock1, sock2;

	sock1.bind(4350);

	sock2.send_to(sendData, IP::loopback(), 4350);

	auto recvData = sock1.recv_from(sendData.size()).data;
	EXPECT_EQ(sendData, recvData);
}

/**
 * @brief Tests basic TCP send and recieve.
 */
TYPED_TEST(SocketTests, TcpSendsAndReceivesData)
{
	using IP = TypeParam;
	auto [sendSock, recvSock] = prepare_tcp<IP>();
	
	const Buffer sendData { byte(1), byte(2), byte(3) };

	sendSock.send_connected(sendData);
	auto recvData = recvSock.recv_connected(sendData.size());
	EXPECT_EQ(sendData, recvData);
}

/**
 * @brief Tests send and recv of strings over TCP.
 */
TYPED_TEST(SocketTests, TcpSendsAndReceivesStrings)
{
	using IP = TypeParam;
	auto [sendSock, recvSock] = prepare_tcp<IP>();

	const std::string sendStr = "abcd";
	const Buffer sendBytes = { 1, 2, 3 };
	sendSock.send_connected_str(sendStr);
	sendSock.send_connected(sendBytes);

	// recieve string with three chars at a time, causing the beggining of next input to be leftover
	auto recvStr = recvSock.template recv_connected_str<std::string, std::endian::big, 3>();

	auto recvBytes = recvSock.recv_connected(3);

	EXPECT_EQ(sendStr, recvStr);
	EXPECT_EQ(sendBytes, recvBytes);

	const std::wstring sendWStr = L"abcd";
	sendSock.send_connected_str(sendWStr);
	sendSock.send_connected(sendBytes);

	// recieve string with three chars at a time, causing the beggining of next input to be leftover
	auto recvWStr = recvSock.template recv_connected_str<std::wstring, std::endian::big, 3>();

	recvBytes = recvSock.recv_connected(3);

	EXPECT_EQ(sendWStr, recvWStr);
	EXPECT_EQ(sendBytes, recvBytes);
}

TYPED_TEST(SocketTests, TcpReceivesExactByteCount)
{
	using IP = TypeParam;
	auto [sendSock, recvSock] = prepare_tcp<IP>();

	const Buffer five = { 1, 2, 3, 4, 5 };
	const Buffer four = { 1, 2, 3, 4 };
	const Buffer last = { 5 };
	sendSock.send_connected(five);
	auto recvFour = recvSock.recv_connected_exact(4);
	EXPECT_EQ(recvFour, four);
	auto recvLast = recvSock.recv_connected(100);
	EXPECT_EQ(recvLast, last);
}

TYPED_TEST(SocketTests, TcpSerializesAndDeserializesTuples)
{
	using IP = TypeParam;
	auto [sendSock, recvSock] = prepare_tcp<IP>();
	auto sendTpl = std::make_tuple(Buffer{1, 2, 3}, std::string("hello"), 5);
	auto recvTpl = std::make_tuple(Buffer(3, byte(0)), std::string(), 0);

	sendSock.send_connected_value(sendTpl);
	recvSock.recv_connected_value(recvTpl);

	EXPECT_EQ(sendTpl, recvTpl);
}

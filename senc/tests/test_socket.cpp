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
 * @brief Tests basic TCP send and recieve.
 */
TEST(SocketTests, TestTCP)
{
	const Buffer sendData { byte(1), byte(2), byte(3) };
	TcpSocket<IPv4> listen_sock, send_sock;
	std::promise<TcpSocket<IPv4>> p;
	std::future<TcpSocket<IPv4>> f = p.get_future();

	listen_sock.bind(4350);
	listen_sock.listen();

	std::jthread t(
		[&listen_sock, &p]()
		{
			try { p.set_value(listen_sock.accept()); }
			catch (...) { p.set_exception(std::current_exception()); }
		}
	);

	send_sock.connect("127.0.0.1", 4350);

	TcpSocket<IPv4> recv_sock = f.get();

	send_sock.send_connected(sendData);
	auto recvData = recv_sock.recv_connected(sendData.size());
	EXPECT_EQ(sendData, recvData);
}

/**
 * @brief Tests send and recv of strings over TCP.
 */
TEST(SocketTests, TestStrTCP)
{
	TcpSocket<IPv4> listen_sock, send_sock;
	std::promise<TcpSocket<IPv4>> p;
	std::future<TcpSocket<IPv4>> f = p.get_future();

	listen_sock.bind(4350);
	listen_sock.listen();

	std::jthread t(
		[&listen_sock, &p]()
		{
			try { p.set_value(listen_sock.accept()); }
			catch (...) { p.set_exception(std::current_exception()); }
		}
	);

	send_sock.connect("127.0.0.1", 4350);

	TcpSocket<IPv4> recv_sock = f.get();

	const std::string send_str = "abcd";
	const Buffer send_bytes = { 1, 2, 3 };
	send_sock.send_connected_str(send_str);
	send_sock.send_connected(send_bytes);

	// recieve string with three chars at a time, causing the beggining of next input to be leftover
	auto recv_str = recv_sock.recv_connected_str<std::string, 3>();

	auto recv_bytes = recv_sock.recv_connected(3);

	EXPECT_EQ(send_str, recv_str);
	EXPECT_EQ(send_bytes, recv_bytes);
}

#include <gtest/gtest.h>
#include <thread>
#include "../utils/Socket.hpp"

using senc::utils::UdpSocket;
using senc::utils::IPv4;
using senc::utils::IPv6;

TEST(SocketTests, TestIPv4)
{
	IPv4 ip("1.2.3.4");
	EXPECT_EQ(ip.as_str(), "1.2.3.4");
}

TEST(SocketTests, TestIPv6)
{
	IPv6 ip("fd30:cb0a:c87a:0157:a1b2:c3d4:e5f6:7890");
	EXPECT_EQ(ip.as_str(), "fd30:cb0a:c87a:0157:a1b2:c3d4:e5f6:7890");
}

TEST(SocketTests, TestUDP)
{
	const std::vector<std::byte> sendData { std::byte(1), std::byte(2), std::byte(3) };
	UdpSocket<IPv4> sock1, sock2;

	sock1.bind(435);
	sock2.bind(436);

	sock2.sendto(sendData, "127.0.0.1", 435);

	auto recvData = sock1.recvfrom(sendData.size(), "127.0.0.1", 436);
	EXPECT_EQ(sendData, recvData);
}

#include <gtest/gtest.h>
#include "../utils/Socket.hpp"

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

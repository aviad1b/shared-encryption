#include <gtest/gtest.h>
#include "../utils/Socket.hpp"

using senc::utils::IPv4;

TEST(SocketTests, TestIPv4)
{
	IPv4 ip("1.2.3.4");
	EXPECT_EQ(ip.as_str(), "1.2.3.4");
}

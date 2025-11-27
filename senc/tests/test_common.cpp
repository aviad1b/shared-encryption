/*********************************************************************
 * \file   test_common.cpp
 * \brief  Contains tests for the "common" unit.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include "tests_utils.hpp"
#include "../common/PacketReceiver.hpp"
#include "../common/PacketSender.hpp"

namespace pkt = senc::pkt;
using senc::PacketReceiver;
using senc::PacketSender;

TEST(TestCommon, ErrorResponse)
{
	auto [client, server] = prepare_tcp();

	pkt::ErrorResponse sendRes{ "this is an error message..." };

	PacketSender::send_request(client, pkt::LogoutRequest{});
	PacketReceiver::recv_request<pkt::LogoutRequest>(server);

	PacketSender::send_response(server, sendRes);
	auto recvRes = PacketReceiver::recv_response<pkt::ErrorResponse>(client);

	EXPECT_EQ(sendRes, recvRes);
}

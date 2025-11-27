/*********************************************************************
 * \file   test_common.cpp
 * \brief  Contains tests for the "common" unit.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include "tests_utils.hpp"
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"

namespace pkt = senc::pkt;
using senc::InlinePacketReceiver;
using senc::InlinePacketSender;

TEST(TestCommon, ErrorResponse)
{
	auto [client, server] = prepare_tcp();

	pkt::ErrorResponse sendRes{ "this is an error message..." };

	InlinePacketSender::send_request(client, pkt::LogoutRequest{});
	InlinePacketReceiver::recv_request<pkt::LogoutRequest>(server);

	InlinePacketSender::send_response(server, sendRes);
	auto recvRes = InlinePacketReceiver::recv_response<pkt::ErrorResponse>(client);

	EXPECT_EQ(sendRes, recvRes);
}

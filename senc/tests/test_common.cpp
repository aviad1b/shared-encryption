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

template <typename Request, typename Response>
void cycle_flow(const Request& req, const Response& resp)
{
	auto [client, server] = prepare_tcp();
	InlinePacketReceiver receiver;
	InlinePacketSender sender;

	sender.send_request(client, req);
	auto reqGot = receiver.recv_request<Request>(server);
	EXPECT_TRUE(reqGot.has_value());
	EXPECT_EQ(reqGot.value(), req);

	sender.send_response(server, resp);
	auto respGot = receiver.recv_response<Response>(client);
	EXPECT_TRUE(respGot.has_value());
	EXPECT_EQ(respGot.value(), resp);
}

TEST(CommonTests, ErrorResponseTest)
{
	pkt::LogoutRequest req{};
	pkt::ErrorResponse resp{ "this is an error message..." };

	cycle_flow(req, resp);
}

TEST(CommonTests, SignupCycleTest)
{
	pkt::SignupRequest req{ "username" };
	pkt::SignupResponse resp{ pkt::SignupResponse::Status::UsernameTaken };

	cycle_flow(req, resp);
}

TEST(CommonTests, LoginCycleTest)
{
	pkt::LoginRequest req{ "username" };
	pkt::LoginResponse resp{ pkt::LoginResponse::Status::BadUsername };

	cycle_flow(req, resp);
}

TEST(CommonTests, LogoutCycleTest)
{
	pkt::LogoutRequest req{};
	pkt::LogoutResponse resp{};

	cycle_flow(req, resp);
}

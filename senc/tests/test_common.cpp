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
using senc::utils::ECGroup;

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

TEST(CommonTests, MakeUserSetCycleTest)
{
	pkt::MakeUserSetRequest req{
		{ "a", "b", "c" },
		{ "o1", "o2", },
		2,
		1
	};

	pkt::MakeUserSetResponse resp{
		"51657d81-1d4b-41ca-9749-cd6ee61cc325",
		ECGroup::identity().pow(435),
		ECGroup::identity().pow(256),
		{ 1, 435 },
		{ 2, 256 }
	};

	cycle_flow(req, resp);
}

TEST(CommonTests, GetUserSetsCycleTest)
{
	pkt::GetUserSetsRequest req{};
	pkt::GetUserSetsResponse resp{
		{
			"51657d81-1d4b-41ca-9749-cd6ee61cc325",
			"c7379469-4294-40b4-850c-fe665717d1ba",
			"57641e16-e02a-473b-8204-a809a9c435df"
		}
	};

	cycle_flow(req, resp);
}

TEST(CommonTests, GetMembersCycleTest)
{
	pkt::GetMembersRequest req{ "51657d81-1d4b-41ca-9749-cd6ee61cc325" };
	pkt::GetMembersResponse resp{ { "a", "asfg", "user" } };

	cycle_flow(req, resp);
}

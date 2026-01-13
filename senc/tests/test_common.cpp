/*********************************************************************
 * \file   test_common.cpp
 * \brief  Contains tests for the "common" unit.
 *
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include "tests_utils.hpp"
#include "../common/InlinePacketHandler.hpp"

namespace pkt = senc::pkt;
using senc::InlinePacketHandler;
using senc::PacketHandler;
using senc::utils::ECGroup;
using senc::utils::Socket;

using PacketHandlerFactory = std::function<std::unique_ptr<PacketHandler>()>;

class PacketsTest : public testing::TestWithParam<PacketHandlerFactory>
{
protected:
	std::unique_ptr<PacketHandler> packetHandler;

	void SetUp() override
	{
		packetHandler = GetParam()();
	}
	
	void TearDown() override
	{
		packetHandler.reset();
	}

public:
	template <typename Request, typename Response>
	void cycle_flow(Socket& client, Socket& server, const Request& req, const Response& resp)
	{
		packetHandler->send_request(client, req);
		auto reqGot = packetHandler->recv_request<Request>(server);
		EXPECT_TRUE(reqGot.has_value());
		EXPECT_EQ(reqGot.value(), req);

		packetHandler->send_response(server, resp);
		auto respGot = packetHandler->recv_response<Response>(client);
		EXPECT_TRUE(respGot.has_value());
		EXPECT_EQ(respGot.value(), resp);
	}
};

static void error_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::LogoutRequest req{};
	pkt::ErrorResponse resp{ "this is an error message..." };
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, ErrorResponseTest)
{
	auto [client, server] = prepare_tcp();
	error_cycle(*this, client, server);
}

static void signup_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::SignupRequest req{ "username", "pass123" };
	pkt::SignupResponse resp{ pkt::SignupResponse::Status::UsernameTaken };
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, SignupCycleTest)
{
	auto [client, server] = prepare_tcp();
	signup_cycle(*this, client, server);
}

static void login_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::LoginRequest req{ "username", "pass123" };
	pkt::LoginResponse resp{ pkt::LoginResponse::Status::BadLogin };
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, LoginCycleTest)
{
	auto [client, server] = prepare_tcp();
	login_cycle(*this, client, server);
}

static void logout_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::LogoutRequest req{};
	pkt::LogoutResponse resp{};
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, LogoutCycleTest)
{
	auto [client, server] = prepare_tcp();
	logout_cycle(*this, client, server);
}

static void make_user_set_cycle(PacketsTest& test, Socket& client, Socket& server)
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
		senc::PrivKeyShard{ 1, 435 },
		senc::PrivKeyShard{ 2, 256 }
	};

	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, MakeUserSetCycleTest)
{
	auto [client, server] = prepare_tcp();
	make_user_set_cycle(*this, client, server);
}

static void get_user_sets_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::GetUserSetsRequest req{};
	pkt::GetUserSetsResponse resp{
		{
			"51657d81-1d4b-41ca-9749-cd6ee61cc325",
			"c7379469-4294-40b4-850c-fe665717d1ba",
			"57641e16-e02a-473b-8204-a809a9c435df"
		}
	};
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, GetUserSetsCycleTest)
{
	auto [client, server] = prepare_tcp();
	get_user_sets_cycle(*this, client, server);
}

static void get_members_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::GetMembersRequest req{ "51657d81-1d4b-41ca-9749-cd6ee61cc325" };
	pkt::GetMembersResponse resp{
		{ "a", "asfg", "user" },
		{ "o1", "o2" }
	};
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, GetMembersCycleTest)
{
	auto [client, server] = prepare_tcp();
	get_members_cycle(*this, client, server);
}

static void decrypt_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::DecryptRequest req{
		"51657d81-1d4b-41ca-9749-cd6ee61cc325",
		{
			ECGroup::identity().pow(435),
			ECGroup::identity().pow(256),
			{
				CryptoPP::SecByteBlock{},
				{ 5, 6, 7, 8, 9 }
			}
		}
	};
	pkt::DecryptResponse resp{ "71f8fdcb-4dbb-4883-a0c2-f99d70b70c34" };
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, DecryptCycleTest)
{
	auto [client, server] = prepare_tcp();
	decrypt_cycle(*this, client, server);
}

static void update_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::UpdateRequest req{};
	pkt::UpdateResponse resp{
		{
			{
				"51657d81-1d4b-41ca-9749-cd6ee61cc325",
				ECGroup::identity().pow(435),
				ECGroup::identity().pow(256),
				senc::PrivKeyShard{ 1, 435 }
			},
			{
				"c7379469-4294-40b4-850c-fe665717d1ba",
				ECGroup::identity().pow(534),
				ECGroup::identity().pow(652),
				senc::PrivKeyShard{ 2, 256 }
			}
		},
		{
			{
				"57641e16-e02a-473b-8204-a809a9c435df",
				ECGroup::identity().pow(111),
				ECGroup::identity().pow(222),
				senc::PrivKeyShard{ 3, 333 },
				senc::PrivKeyShard{ 13, 131313 }
			},
			{
				"55b27150-1668-446f-aa50-35d9358eac19",
				ECGroup::identity().pow(444),
				ECGroup::identity().pow(555),
				senc::PrivKeyShard{ 4, 666 },
				senc::PrivKeyShard{ 14, 161616 }
			}
		},
		{
			"71f8fdcb-4dbb-4883-a0c2-f99d70b70c34",
			"0db2e378-9fdb-4f2a-8ea6-df3e1e9a9d2c"
		},
		{
			{
				"663383cf-d302-4eaf-8680-e8abcf240d89",
				{
					ECGroup::identity().pow(5),
					ECGroup::identity().pow(6),
					{
						CryptoPP::SecByteBlock{},
						{ 5, 6, 7, 8, 9 }
					}
				},
				{ 1, 2, 3, 4 }
			},
			{
				"1349f2e2-df59-4a4e-82c5-a74e009a72f0",
				{
					ECGroup::identity().pow(43),
					ECGroup::identity().pow(56),
					{
						CryptoPP::SecByteBlock{},
						{ 8, 8, 8, 8, 8 }
					}
				},
				{ 5, 6, 7, 8 }
			}
		},
		{
			{
				"07c039b6-5a7c-4a3c-9a7a-85ff31710f2f",
				{ ECGroup::identity().pow(3), ECGroup::identity().pow(4) },
				{ ECGroup::identity().pow(5), ECGroup::identity().pow(6) },
				{ 1, 2, 100 },
				{ 3, 4, 100 }
			},
			{
				"d26af60a-0971-4916-898d-54cb02097333",
				{ ECGroup::identity().pow(8) },
				{ },
				{ 5, 100 },
				{ 100 }
			}
		}
	};
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, UpdateCycleTest)
{
	auto [client, server] = prepare_tcp();
	update_cycle(*this, client, server);
}

static void decrypt_participate_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::DecryptParticipateRequest req{ "71f8fdcb-4dbb-4883-a0c2-f99d70b70c34" };
	pkt::DecryptParticipateResponse resp{ pkt::DecryptParticipateResponse::Status::NotRequired };
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, DecryptParticipateCycleTest)
{
	auto [client, server] = prepare_tcp();
	decrypt_participate_cycle(*this, client, server);
}

static void send_decryption_part_cycle(PacketsTest& test, Socket& client, Socket& server)
{
	pkt::SendDecryptionPartRequest req{
		"71f8fdcb-4dbb-4883-a0c2-f99d70b70c34",
		ECGroup::identity().pow(435)
	};
	pkt::SendDecryptionPartResponse resp{};
	test.cycle_flow(client, server, req, resp);
}

TEST_P(PacketsTest, SendDecryptionPartCycleTest)
{
	auto [client, server] = prepare_tcp();
	send_decryption_part_cycle(*this, client, server);
}

TEST_P(PacketsTest, AllProtocolCyclesInSequence)
{
	auto [client, server] = prepare_tcp();
	error_cycle(*this, client, server);
	signup_cycle(*this, client, server);
	login_cycle(*this, client, server);
	make_user_set_cycle(*this, client, server);
	get_user_sets_cycle(*this, client, server);
	get_members_cycle(*this, client, server);
	decrypt_cycle(*this, client, server);
	update_cycle(*this, client, server);
	decrypt_participate_cycle(*this, client, server);
	send_decryption_part_cycle(*this, client, server);
	logout_cycle(*this, client, server);
}

TEST_P(PacketsTest, LoginWithErrorsCycleTest)
{
	auto [client, server] = prepare_tcp();

	pkt::LoginRequest req{ "username", "pass123" };
	pkt::LoginResponse loginResp{ pkt::LoginResponse::Status::BadLogin };
	pkt::ErrorResponse errResp{ "Some error message" };
	pkt::LogoutResponse logoutResp{};

	packetHandler->send_request(client, req);
	auto reqGot1 = packetHandler->recv_request<pkt::LoginRequest>(server);
	EXPECT_TRUE(reqGot1.has_value());
	EXPECT_EQ(reqGot1.value(), req);

	packetHandler->send_response(server, errResp);
	auto respGot1 = packetHandler->recv_response<pkt::LoginResponse, pkt::ErrorResponse>(client);
	EXPECT_TRUE(respGot1.has_value());
	EXPECT_TRUE(std::holds_alternative<pkt::ErrorResponse>(*respGot1));
	EXPECT_EQ(std::get<pkt::ErrorResponse>(*respGot1), errResp);

	packetHandler->send_request(client, req);
	auto reqGot2 = packetHandler->recv_request<pkt::LoginRequest>(server);
	EXPECT_TRUE(reqGot2.has_value());
	EXPECT_EQ(reqGot2.value(), req);

	packetHandler->send_response(server, logoutResp);
	auto respGot2 = packetHandler->recv_response<pkt::LoginResponse, pkt::ErrorResponse>(client);
	EXPECT_FALSE(respGot2.has_value());

	packetHandler->send_request(client, req);
	auto reqGot3 = packetHandler->recv_request<pkt::LoginRequest>(server);
	EXPECT_TRUE(reqGot3.has_value());
	EXPECT_EQ(reqGot3.value(), req);

	packetHandler->send_response(server, loginResp);
	auto respGot3 = packetHandler->recv_response<pkt::LoginResponse, pkt::ErrorResponse>(client);
	EXPECT_TRUE(respGot3.has_value());
	EXPECT_TRUE(std::holds_alternative<pkt::LoginResponse>(*respGot3));
	EXPECT_EQ(std::get<pkt::LoginResponse>(*respGot3), loginResp);
}

TEST_P(PacketsTest, TestRequestVariant)
{
	auto [client, server] = prepare_tcp();

	pkt::SignupRequest signupReq{ "username", "pass123" };
	pkt::LoginRequest loginReq{ "AAAAAAAA", "pass123" };
	pkt::LogoutRequest logoutReq{};

	packetHandler->send_request(client, signupReq);
	auto reqGot1 = packetHandler->recv_request<pkt::SignupRequest, pkt::LoginRequest>(server);
	EXPECT_TRUE(reqGot1.has_value());
	EXPECT_TRUE(std::holds_alternative<pkt::SignupRequest>(*reqGot1));
	EXPECT_EQ(std::get<pkt::SignupRequest>(*reqGot1), signupReq);

	packetHandler->send_request(client, loginReq);
	auto reqGot2 = packetHandler->recv_request<pkt::SignupRequest, pkt::LoginRequest>(server);
	EXPECT_TRUE(reqGot2.has_value());
	EXPECT_TRUE(std::holds_alternative<pkt::LoginRequest>(*reqGot2));
	EXPECT_EQ(std::get<pkt::LoginRequest>(*reqGot2), loginReq);

	packetHandler->send_request(client, logoutReq);
	auto reqGot3 = packetHandler->recv_request<pkt::SignupRequest, pkt::LoginRequest>(server);
	EXPECT_FALSE(reqGot3.has_value());
}

INSTANTIATE_TEST_SUITE_P(
	PacketTests,
	PacketsTest,
	testing::Values(
		std::make_unique<InlinePacketHandler>
	)
);

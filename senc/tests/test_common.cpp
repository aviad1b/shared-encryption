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
using senc::utils::Socket;

template <typename Request, typename Response>
void cycle_flow(Socket& client, Socket& server, const Request& req, const Response& resp)
{
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

static void error_cycle(Socket& client, Socket& server)
{
	pkt::LogoutRequest req{};
	pkt::ErrorResponse resp{ "this is an error message..." };
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, ErrorResponseTest)
{
	auto [client, server] = prepare_tcp();
	error_cycle(client, server);
}

static void signup_cycle(Socket& client, Socket& server)
{
	pkt::SignupRequest req{ "username" };
	pkt::SignupResponse resp{ pkt::SignupResponse::Status::UsernameTaken };
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, SignupCycleTest)
{
	auto [client, server] = prepare_tcp();
	signup_cycle(client, server);
}

static void login_cycle(Socket& client, Socket& server)
{
	pkt::LoginRequest req{ "username" };
	pkt::LoginResponse resp{ pkt::LoginResponse::Status::BadUsername };
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, LoginCycleTest)
{
	auto [client, server] = prepare_tcp();
	login_cycle(client, server);
}

static void logout_cycle(Socket& client, Socket& server)
{
	pkt::LogoutRequest req{};
	pkt::LogoutResponse resp{};
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, LogoutCycleTest)
{
	auto [client, server] = prepare_tcp();
	logout_cycle(client, server);
}

static void make_user_set_cycle(Socket& client, Socket& server)
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

	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, MakeUserSetCycleTest)
{
	auto [client, server] = prepare_tcp();
	make_user_set_cycle(client, server);
}

static void get_user_sets_cycle(Socket& client, Socket& server)
{
	pkt::GetUserSetsRequest req{};
	pkt::GetUserSetsResponse resp{
		{
			"51657d81-1d4b-41ca-9749-cd6ee61cc325",
			"c7379469-4294-40b4-850c-fe665717d1ba",
			"57641e16-e02a-473b-8204-a809a9c435df"
		}
	};
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, GetUserSetsCycleTest)
{
	auto [client, server] = prepare_tcp();
	get_user_sets_cycle(client, server);
}

static void get_members_cycle(Socket& client, Socket& server)
{
	pkt::GetMembersRequest req{ "51657d81-1d4b-41ca-9749-cd6ee61cc325" };
	pkt::GetMembersResponse resp{
		{ "a", "asfg", "user" },
		{ "o1", "o2" }
	};
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, GetMembersCycleTest)
{
	auto [client, server] = prepare_tcp();
	get_members_cycle(client, server);
}

static void decrypt_cycle(Socket& client, Socket& server)
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
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, DecryptCycleTest)
{
	auto [client, server] = prepare_tcp();
	decrypt_cycle(client, server);
}

static void update_cycle(Socket& client, Socket& server)
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
				"2de36b23-5e7f-4d0a-aa2f-22641164006c",
				{ ECGroup::identity().pow(3), ECGroup::identity().pow(4) },
				{ ECGroup::identity().pow(5), ECGroup::identity().pow(6) },
			},
			{
				"d26af60a-0971-4916-898d-54cb02097333",
				"75c2fbfe-30a5-4598-8bc7-211ce0dca5b8",
				{ ECGroup::identity().pow(8) },
				{ },
			}
		}
	};
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, UpdateCycleTest)
{
	auto [client, server] = prepare_tcp();
	update_cycle(client, server);
}

static void decrypt_participate_cycle(Socket& client, Socket& server)
{
	pkt::DecryptParticipateRequest req{ "71f8fdcb-4dbb-4883-a0c2-f99d70b70c34" };
	pkt::DecryptParticipateResponse resp{ pkt::DecryptParticipateResponse::Status::NotRequired };
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, DecryptParticipateCycleTest)
{
	auto [client, server] = prepare_tcp();
	decrypt_participate_cycle(client, server);
}

static void send_decryption_part_cycle(Socket& client, Socket& server)
{
	pkt::SendDecryptionPartRequest req{
		"71f8fdcb-4dbb-4883-a0c2-f99d70b70c34",
		ECGroup::identity().pow(435)
	};
	pkt::SendDecryptionPartResponse resp{};
	cycle_flow(client, server, req, resp);
}

TEST(CommonTests, SendDecryptionPartCycleTest)
{
	auto [client, server] = prepare_tcp();
	send_decryption_part_cycle(client, server);
}

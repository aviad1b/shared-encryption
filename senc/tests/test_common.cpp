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
#include "../common/EncryptedPacketHandler.hpp"
#include "../common/PacketHandlerFactory.hpp"
#include "../common/InlinePacketHandler.hpp"

namespace pkt = senc::pkt;
using senc::PacketHandlerImplFactory;
using senc::EncryptedPacketHandler;
using senc::PacketHandlerFactory;
using senc::InlinePacketHandler;
using senc::PacketHandler;
using senc::utils::ECGroup;
using senc::utils::Socket;

class PacketsTest : public testing::TestWithParam<PacketHandlerFactory>
{
protected:
	std::unique_ptr<PacketHandler> clientPacketHandler, serverPacketHandler;
	std::unique_ptr<Socket> client, server;

	void SetUp() override
	{
		auto& packetHandlerFactory = GetParam();
		auto [client, server] = prepare_tcp();
		this->client = std::make_unique<decltype(client)>(std::move(client));
		this->server = std::make_unique<decltype(server)>(std::move(server));
		std::tie(clientPacketHandler, serverPacketHandler) =
			prepare_for_sockets<std::unique_ptr<PacketHandler>>(
				*this->client, [&packetHandlerFactory](Socket& sock)
				{
					return packetHandlerFactory.new_client_packet_handler(sock);
				},
				*this->server, [&packetHandlerFactory](Socket& sock)
				{
					return packetHandlerFactory.new_server_packet_handler(sock);
				}
			);
	}
	
	void TearDown() override
	{
		client.reset();
		server.reset();
		clientPacketHandler.reset();
		serverPacketHandler.reset();
	}

public:
	template <typename Request, typename Response>
	void cycle_flow(const Request& req, const Response& resp)
	{
		clientPacketHandler->send_request(req);
		auto reqGot = serverPacketHandler->recv_request<Request>();
		EXPECT_TRUE(reqGot.has_value());
		EXPECT_EQ(reqGot.value(), req);

		serverPacketHandler->send_response(resp);
		auto respGot = clientPacketHandler->recv_response<Response>();
		EXPECT_TRUE(respGot.has_value());
		EXPECT_EQ(respGot.value(), resp);
	}
};

static void error_cycle(PacketsTest& test)
{
	pkt::LogoutRequest req{};
	pkt::ErrorResponse resp{ "this is an error message..." };
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, ErrorResponseTest)
{
	error_cycle(*this);
}

static void signup_cycle(PacketsTest& test)
{
	pkt::SignupRequest req{ "username", "pass123" };
	pkt::SignupResponse resp{ pkt::SignupResponse::Status::UsernameTaken };
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, SignupCycleTest)
{
	signup_cycle(*this);
}

static void login_cycle(PacketsTest& test)
{
	pkt::LoginRequest req{ "username", "pass123" };
	pkt::LoginResponse resp{ pkt::LoginResponse::Status::BadLogin };
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, LoginCycleTest)
{
	login_cycle(*this);
}

static void logout_cycle(PacketsTest& test)
{
	pkt::LogoutRequest req{};
	pkt::LogoutResponse resp{};
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, LogoutCycleTest)
{
	logout_cycle(*this);
}

static void make_user_set_cycle(PacketsTest& test)
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

	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, MakeUserSetCycleTest)
{
	make_user_set_cycle(*this);
}

static void get_user_sets_cycle(PacketsTest& test)
{
	pkt::GetUserSetsRequest req{};
	pkt::GetUserSetsResponse resp{
		{
			"51657d81-1d4b-41ca-9749-cd6ee61cc325",
			"c7379469-4294-40b4-850c-fe665717d1ba",
			"57641e16-e02a-473b-8204-a809a9c435df"
		}
	};
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, GetUserSetsCycleTest)
{
	get_user_sets_cycle(*this);
}

static void get_members_cycle(PacketsTest& test)
{
	pkt::GetMembersRequest req{ "51657d81-1d4b-41ca-9749-cd6ee61cc325" };
	pkt::GetMembersResponse resp{
		{ "a", "asfg", "user" },
		{ "o1", "o2" }
	};
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, GetMembersCycleTest)
{
	get_members_cycle(*this);
}

static void decrypt_cycle(PacketsTest& test)
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
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, DecryptCycleTest)
{
	decrypt_cycle(*this);
}

static void update_cycle(PacketsTest& test)
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
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, UpdateCycleTest)
{
	update_cycle(*this);
}

static void decrypt_participate_cycle(PacketsTest& test)
{
	pkt::DecryptParticipateRequest req{ "71f8fdcb-4dbb-4883-a0c2-f99d70b70c34" };
	pkt::DecryptParticipateResponse resp{ pkt::DecryptParticipateResponse::Status::NotRequired };
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, DecryptParticipateCycleTest)
{
	decrypt_participate_cycle(*this);
}

static void send_decryption_part_cycle(PacketsTest& test)
{
	pkt::SendDecryptionPartRequest req{
		"71f8fdcb-4dbb-4883-a0c2-f99d70b70c34",
		ECGroup::identity().pow(435)
	};
	pkt::SendDecryptionPartResponse resp{};
	test.cycle_flow(req, resp);
}

TEST_P(PacketsTest, SendDecryptionPartCycleTest)
{
	send_decryption_part_cycle(*this);
}

TEST_P(PacketsTest, AllProtocolCyclesInSequence)
{
	error_cycle(*this);
	signup_cycle(*this);
	login_cycle(*this);
	make_user_set_cycle(*this);
	get_user_sets_cycle(*this);
	get_members_cycle(*this);
	decrypt_cycle(*this);
	update_cycle(*this);
	decrypt_participate_cycle(*this);
	send_decryption_part_cycle(*this);
	logout_cycle(*this);
}

TEST_P(PacketsTest, LoginWithErrorsCycleTest)
{
	pkt::LoginRequest req{ "username", "pass123" };
	pkt::LoginResponse loginResp{ pkt::LoginResponse::Status::BadLogin };
	pkt::ErrorResponse errResp{ "Some error message" };
	pkt::LogoutResponse logoutResp{};

	clientPacketHandler->send_request(req);
	auto reqGot1 = serverPacketHandler->recv_request<pkt::LoginRequest>();
	EXPECT_TRUE(reqGot1.has_value());
	EXPECT_EQ(reqGot1.value(), req);

	serverPacketHandler->send_response(errResp);
	auto respGot1 = clientPacketHandler->recv_response<pkt::LoginResponse, pkt::ErrorResponse>();
	EXPECT_TRUE(respGot1.has_value());
	EXPECT_TRUE(std::holds_alternative<pkt::ErrorResponse>(*respGot1));
	EXPECT_EQ(std::get<pkt::ErrorResponse>(*respGot1), errResp);

	clientPacketHandler->send_request(req);
	auto reqGot2 = serverPacketHandler->recv_request<pkt::LoginRequest>();
	EXPECT_TRUE(reqGot2.has_value());
	EXPECT_EQ(reqGot2.value(), req);

	serverPacketHandler->send_response(logoutResp);
	auto respGot2 = clientPacketHandler->recv_response<pkt::LoginResponse, pkt::ErrorResponse>();
	EXPECT_FALSE(respGot2.has_value());

	clientPacketHandler->send_request(req);
	auto reqGot3 = serverPacketHandler->recv_request<pkt::LoginRequest>();
	EXPECT_TRUE(reqGot3.has_value());
	EXPECT_EQ(reqGot3.value(), req);

	serverPacketHandler->send_response(loginResp);
	auto respGot3 = clientPacketHandler->recv_response<pkt::LoginResponse, pkt::ErrorResponse>();
	EXPECT_TRUE(respGot3.has_value());
	EXPECT_TRUE(std::holds_alternative<pkt::LoginResponse>(*respGot3));
	EXPECT_EQ(std::get<pkt::LoginResponse>(*respGot3), loginResp);
}

TEST_P(PacketsTest, TestRequestVariant)
{
	pkt::SignupRequest signupReq{ "username", "pass123" };
	pkt::LoginRequest loginReq{ "AAAAAAAA", "pass123" };
	pkt::LogoutRequest logoutReq{};

	clientPacketHandler->send_request(signupReq);
	auto reqGot1 = serverPacketHandler->recv_request<pkt::SignupRequest, pkt::LoginRequest>();
	EXPECT_TRUE(reqGot1.has_value());
	EXPECT_TRUE(std::holds_alternative<pkt::SignupRequest>(*reqGot1));
	EXPECT_EQ(std::get<pkt::SignupRequest>(*reqGot1), signupReq);

	clientPacketHandler->send_request(loginReq);
	auto reqGot2 = serverPacketHandler->recv_request<pkt::SignupRequest, pkt::LoginRequest>();
	EXPECT_TRUE(reqGot2.has_value());
	EXPECT_TRUE(std::holds_alternative<pkt::LoginRequest>(*reqGot2));
	EXPECT_EQ(std::get<pkt::LoginRequest>(*reqGot2), loginReq);

	clientPacketHandler->send_request(logoutReq);
	auto reqGot3 = serverPacketHandler->recv_request<pkt::SignupRequest, pkt::LoginRequest>();
	EXPECT_FALSE(reqGot3.has_value());
}

INSTANTIATE_TEST_SUITE_P(
	PacketTests,
	PacketsTest,
	testing::Values(
		PacketHandlerImplFactory<InlinePacketHandler>{},
		PacketHandlerImplFactory<EncryptedPacketHandler>{}
	)
);

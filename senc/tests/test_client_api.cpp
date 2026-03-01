/*********************************************************************
 * \file   test_client_api.cpp
 * \brief  Contains tests for client API.
 * 
 * \author aviad1b
 * \date   March 2026, Adar 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include "tests_utils.hpp"
#include "../server/storage/ShortTermServerStorage.hpp"
#include "../common/EncryptedPacketHandler.hpp"
#include "../common/EncryptedPacketHandler.hpp"
#include "../client_api/client_api.h"
#include "../server/Server.hpp"

using senc::server::storage::ShortTermServerStorage;
using senc::server::managers::DecryptionsManager;
using senc::server::managers::UpdateManager;
using senc::server::storage::IServerStorage;
using senc::ServerPacketHandlerImplFactory;
using senc::EncryptedPacketHandler;
using senc::server::IServer;
using senc::server::Server;
using senc::utils::Port;
using senc::utils::IPv4;
using senc::Schema;

class ClientApiTest : public testing::Test
{
protected:
	Port port;
	Schema serverSchema;
	UpdateManager updateManager;
	DecryptionsManager decryptionsManager;
	std::unique_ptr<IServerStorage> serverStorage;
	std::unique_ptr<IServer> server;
	const char* ip;

	void SetUp() override
	{
		serverStorage = std::make_unique<ShortTermServerStorage>();
		server = new_server<IPv4>(
			serverSchema,
			*serverStorage,
			ServerPacketHandlerImplFactory<EncryptedPacketHandler>{},
			updateManager,
			decryptionsManager
		);
		server->start();
		port = server->port();
		ip = IPv4::loopback().as_str().c_str();
	}

	void TearDown() override
	{
		server->stop();
		serverStorage.reset();
		server.reset();
	}
};

TEST_F(ClientApiTest, RoundTripFlow)
{

}

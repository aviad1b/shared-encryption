#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include "../utils/Socket.hpp" // has to be first because windows is stupid
#include "../server/ShortTermServerStorage.hpp"
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "../server/Server.hpp"

using senc::server::ShortTermServerStorage;
using senc::server::DecryptionsManager;
using senc::server::IServerStorage;
using senc::server::UpdateManager;
using senc::InlinePacketReceiver;
using senc::InlinePacketSender;
using senc::server::Server;
using senc::PacketReceiver;
using senc::PacketSender;
using senc::utils::Port;
using senc::Schema;

using Socket = senc::utils::TcpSocket<senc::utils::IPv4>;

struct ServerTestParams
{
	Port port;
	Schema schema;
	std::unique_ptr<IServerStorage> storage;
	std::unique_ptr<PacketReceiver> receiver;
	std::unique_ptr<PacketSender> sender;
	UpdateManager updateManager;
	DecryptionsManager decryptionsManager;
};

class ServerTest : public testing::TestWithParam<ServerTestParams>
{
protected:
	std::unique_ptr<Server> server;

	void SetUp() override
	{
		const auto& params = GetParam();
		server = std::make_unique<Server>(
			params.port,
			params.schema,
			*params.storage,
			*params.receiver,
			*params.sender,
			params.updateManager,
			params.decryptionsManager
		);
		server->start();
	}

	void TearDown() override
	{
		server->stop();
		server.reset();
	}
};

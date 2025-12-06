#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include "../utils/Socket.hpp" // has to be first because windows is stupid
#include "../server/ShortTermServerStorage.hpp"
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "../server/Server.hpp"
#include "../utils/Random.hpp"

namespace pkt = senc::pkt;
using senc::server::ShortTermServerStorage;
using senc::server::DecryptionsManager;
using senc::server::IServerStorage;
using senc::server::UpdateManager;
using senc::InlinePacketReceiver;
using senc::InlinePacketSender;
using senc::server::Server;
using senc::PacketReceiver;
using senc::utils::Random;
using senc::PacketSender;
using senc::utils::Port;
using senc::Schema;

using Socket = senc::utils::TcpSocket<senc::utils::IPv4>;

// factory function types for creating member implementations
using StorageFactory = std::function<std::unique_ptr<IServerStorage>()>;
using ReceiverFactory = std::function<std::unique_ptr<PacketReceiver>()>;
using SenderFactory = std::function<std::unique_ptr<PacketSender>()>;

struct ServerTestParams
{
	StorageFactory storageFactory;
	ReceiverFactory receiverFactory;
	SenderFactory senderFactory;
};

class ServerTest : public testing::TestWithParam<ServerTestParams>
{
protected:
	Port port;
	Schema schema;
	UpdateManager updateManager;
	DecryptionsManager decryptionsManager;
	std::unique_ptr<IServerStorage> storage;
	std::unique_ptr<PacketReceiver> receiver;
	std::unique_ptr<PacketSender> sender;
	std::unique_ptr<Server> server;

	void SetUp() override
	{
		port = Random<Port>::sample_from_range(49152, 65535);
		auto& params = GetParam();
		storage = params.storageFactory();
		receiver = params.receiverFactory();
		sender = params.senderFactory();
		server = std::make_unique<Server>(
			port,
			schema,
			*storage,
			*receiver,
			*sender,
			updateManager,
			decryptionsManager
		);
		server->start();
	}

	void TearDown() override
	{
		server->stop();
		storage.reset();
		receiver.reset();
		sender.reset();
		server.reset();
	}

	template <typename Response>
	auto post(senc::utils::Socket& sock, const auto& request) const
	{
		sender->send_request(sock, request);
		return receiver->recv_response<Response>(sock);
	}
};

TEST_P(ServerTest, SignupAndGetUsers)
{
	auto avi = Socket("127.0.0.1", port);
	auto batya = Socket("127.0.0.1", port);

	auto r1 = post<pkt::SignupResponse>(avi, pkt::SignupRequest{ "avi" });
	EXPECT_TRUE(r1.has_value() && r1->status == pkt::SignupResponse::Status::Success);
	auto r2 = post<pkt::SignupResponse>(batya, pkt::SignupRequest{ "batya" });
	EXPECT_TRUE(r2.has_value() && r2->status == pkt::SignupResponse::Status::Success);

	EXPECT_TRUE(storage->user_exists("avi"));
	EXPECT_TRUE(storage->user_exists("batya"));
}

// ===== Instantiation of Parameterized Tests =====

INSTANTIATE_TEST_SUITE_P(
	ServerImplementations,
	ServerTest,
	testing::Values(
		ServerTestParams{
			std::make_unique<ShortTermServerStorage>,
			std::make_unique<InlinePacketReceiver>,
			std::make_unique<InlinePacketSender>
		}
	)
);

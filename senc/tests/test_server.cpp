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
using senc::DecryptionPart;
using senc::utils::Random;
using senc::utils::Buffer;
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

TEST_P(ServerTest, ImmediateLogout)
{
	auto client = Socket("127.0.0.1", port);
	auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
	EXPECT_TRUE(lo.has_value());
}

TEST_P(ServerTest, SignupAndLogin)
{
	auto avi = Socket("127.0.0.1", port);
	auto batya = Socket("127.0.0.1", port);

	// signup
	auto su1 = post<pkt::SignupResponse>(avi, pkt::SignupRequest{ "avi" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(batya, pkt::SignupRequest{ "batya" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);

	// check users exist
	EXPECT_TRUE(storage->user_exists("avi"));
	EXPECT_TRUE(storage->user_exists("batya"));

	// logout
	auto lo1 = post<pkt::LogoutResponse>(avi, pkt::LogoutRequest{});
	EXPECT_TRUE(lo1.has_value());
	auto lo2 = post<pkt::LogoutResponse>(batya, pkt::LogoutRequest{});
	EXPECT_TRUE(lo2.has_value());

	// log back in
	avi = Socket("127.0.0.1", port);
	batya = Socket("127.0.0.1", port);
	auto li1 = post<pkt::LoginResponse>(avi, pkt::LoginRequest{ "avi" });
	EXPECT_TRUE(li1.has_value() && li1->status == pkt::LoginResponse::Status::Success);
	auto li2 = post<pkt::LoginResponse>(batya, pkt::LoginRequest{ "batya" });
	EXPECT_TRUE(li2.has_value() && li2->status == pkt::LoginResponse::Status::Success);

	// logout
	lo1 = post<pkt::LogoutResponse>(avi, pkt::LogoutRequest{});
	EXPECT_TRUE(lo1.has_value());
	lo2 = post<pkt::LogoutResponse>(batya, pkt::LogoutRequest{});
	EXPECT_TRUE(lo2.has_value());
}

TEST_P(ServerTest, MakeSetGetMembers)
{
	auto client1 = Socket("127.0.0.1", port);
	auto client2 = Socket("127.0.0.1", port);
	auto client3 = Socket("127.0.0.1", port);

	// signup
	const auto u1 = "avi";
	auto su1 = post<pkt::SignupResponse>(client1, pkt::SignupRequest{ u1 });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	const auto u2 = "batya";
	auto su2 = post<pkt::SignupResponse>(client2, pkt::SignupRequest{ u2 });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	const auto u3 = "gal";
	auto su3 = post<pkt::SignupResponse>(client3, pkt::SignupRequest{ u3 });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set
	auto ms = post<pkt::MakeUserSetResponse>(client1, pkt::MakeUserSetRequest{
		.reg_members = { u2 },
		.owners = { u3 },
		.reg_members_threshold = 1,
		.owners_threshold = 1
	});
	EXPECT_TRUE(ms.has_value());
	const auto& usersetID = ms->user_set_id;

	// for each owner (u1 and u3)
	for (auto& client : { std::ref(client1), std::ref(client3) })
	{
		// get sets
		auto gs = post<pkt::GetUserSetsResponse>(client, pkt::GetUserSetsRequest{});
		EXPECT_TRUE(gs.has_value());

		// check that `usersetID` is in sets
		EXPECT_NE(
			std::find(
				gs->user_sets_ids.begin(),
				gs->user_sets_ids.end(),
				usersetID
			),
			gs->user_sets_ids.end()
		);

		// get members
		auto gm = post<pkt::GetMembersResponse>(client, pkt::GetMembersRequest{ usersetID });
		EXPECT_TRUE(gm.has_value());

		// check that u1 and u3 are owners
		for (const auto& owner : { u1, u3 })
			EXPECT_NE(
				std::find(
					gm->owners.begin(),
					gm->owners.end(),
					owner
				),
				gm->owners.end()
			);

		// check that u2 is a (regular) member
		EXPECT_NE(
			std::find(
				gm->reg_members.begin(),
				gm->reg_members.end(),
				u2
			),
			gm->reg_members.end()
		);
	}

	// logout
	for (auto& client : { std::ref(client1), std::ref(client2), std::ref(client3) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, EmptyUpdateCycle)
{
	auto client = Socket("127.0.0.1", port);

	// signup
	auto su = post<pkt::SignupResponse>(client, pkt::SignupRequest{ "avi" });
	EXPECT_TRUE(su.has_value() && su->status == pkt::SignupResponse::Status::Success);

	// update
	auto up = post<pkt::UpdateResponse>(client, pkt::UpdateRequest{});
	EXPECT_TRUE(up.has_value());

	EXPECT_TRUE(up->added_as_reg_member.empty());
	EXPECT_TRUE(up->added_as_owner.empty());
	EXPECT_TRUE(up->to_decrypt.empty());
	EXPECT_TRUE(up->finished_decryptions.empty());

	// logout
	auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
	EXPECT_TRUE(lo.has_value());
}

TEST_P(ServerTest, DecryptFlowSimple)
{
	auto owner = Socket("127.0.0.1", port);
	auto member = Socket("127.0.0.1", port);

	// signup
	auto su1 = post<pkt::SignupResponse>(owner, pkt::SignupRequest{ "owner" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(member, pkt::SignupRequest{ "member" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=1
	auto ms = post<pkt::MakeUserSetResponse>(owner, pkt::MakeUserSetRequest{
		.reg_members = { "member" },
		.owners = { },
		.reg_members_threshold = 1,
		.owners_threshold = 0
	});
	EXPECT_TRUE(ms.has_value());
	const auto& ownerUsersetID = ms->user_set_id;
	const auto& ownerPubKey1 = ms->pub_key1;
	const auto& ownerPubKey2 = ms->pub_key2;
	const auto& ownerShard1 = ms->priv_key1_shard;
	const auto& ownerShard2 = ms->priv_key2_shard;

	// encrypt a message
	Schema schema;
	const std::string msgStr = "Hello There";
	const Buffer msg(msgStr.begin(), msgStr.end());
	auto ownerCiphertext = schema.encrypt(msg, ownerPubKey1, ownerPubKey2);

	// 1) owner starts decryption
	auto dc = post<pkt::DecryptResponse>(owner, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) member runs update to get decryption lookup request
	auto up1 = post<pkt::UpdateResponse>(member, pkt::UpdateRequest{});
	EXPECT_TRUE(up1.has_value());
	const auto& memberSetsAddedTo = up1->added_as_reg_member;
	const auto& memberOnLookup = up1->on_lookup;

	//    member was added to one set, check same as owner's
	EXPECT_EQ(memberSetsAddedTo.size(), 1);
	EXPECT_EQ(memberSetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(memberSetsAddedTo.front().pub_key1, ownerPubKey1);
	EXPECT_EQ(memberSetsAddedTo.front().pub_key2, ownerPubKey2);
	const auto& memberShard = memberSetsAddedTo.front().priv_key1_shard;

	//    member has one operation to participate in, check same as owner's
	EXPECT_EQ(memberOnLookup.size(), 1);
	EXPECT_EQ(memberOnLookup.front(), ownerOpid);

	// 3) member tells server that they're willing to participate in operation
	auto dp = post<pkt::DecryptParticipateResponse>(member, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp.has_value() && dp->status == pkt::DecryptParticipateResponse::Status::SendPart);

	// 4) member runs update to get decryption request
	auto up2 = post<pkt::UpdateResponse>(member, pkt::UpdateRequest{});
	EXPECT_TRUE(up2.has_value());
	const auto& memberToDecrypt = up2->to_decrypt;

	//    member has one part to decrypt, check same operation as owner
	EXPECT_EQ(memberToDecrypt.size(), 1);
	const auto& memberOpid = memberToDecrypt.front().op_id;
	const auto& memberCiphertext = memberToDecrypt.front().ciphertext;
	const auto& memberShardsIDs = memberToDecrypt.front().shards_ids;
	EXPECT_EQ(memberOpid, ownerOpid);
	EXPECT_EQ(memberCiphertext, ownerCiphertext);

	// 5) member computes decryption part locally
	auto memberPart = senc::Shamir::decrypt_get_2l<1>(
		memberCiphertext,
		memberShard,
		memberShardsIDs
	);
	// (member knows it's not an owner, so layer 1)

	// 6) member sends decryption part back
	auto sp = post<pkt::SendDecryptionPartResponse>(member, pkt::SendDecryptionPartRequest{
		.op_id = memberOpid,
		.decryption_part = memberPart
	});
	EXPECT_TRUE(sp.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(owner, pkt::UpdateRequest{});
	EXPECT_TRUE(up3.has_value());

	//    member has one finished decrytion, check same as submitted
	const auto& finished = up3->finished_decryptions;
	EXPECT_EQ(finished.size(), 1);
	EXPECT_EQ(finished.front().op_id, ownerOpid);
	const auto& finishedShardsIDs1 = finished.front().shardsIDs1;
	const auto& finishedShardsIDs2 = finished.front().shardsIDs2;
	const auto& finishedParts1 = finished.front().parts1;
	const auto& finishedParts2 = finished.front().parts2;

	// 8) owner computes their own decryption parts
	auto ownerPart1 = senc::Shamir::decrypt_get_2l<1>(
		ownerCiphertext,
		ownerShard1,
		finishedShardsIDs1
	);
	auto ownerPart2 = senc::Shamir::decrypt_get_2l<2>(
		ownerCiphertext,
		ownerShard2,
		finishedShardsIDs2
	);

	// 9) owner combines their parts with received parts and decrypts fully
	std::vector<DecryptionPart> parts1 = finishedParts1;
	parts1.push_back(ownerPart1);
	std::vector<DecryptionPart> parts2 = finishedParts2;
	parts2.push_back(ownerPart2);
	auto decrypted = senc::Shamir::decrypt_join_2l(
		ownerCiphertext, parts1, parts2
	);
	EXPECT_EQ(decrypted, msg);

	// logout
	for (auto& client : { std::ref(owner), std::ref(member) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
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

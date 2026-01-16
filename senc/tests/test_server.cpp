/*********************************************************************
 * \file   test_server.cpp
 * \brief  Contains tests for server.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include "../utils/Socket.hpp" // has to be first because windows is stupid
#include "tests_utils.hpp"
#include "../server/ShortTermServerStorage.hpp"
#include "../common/EncryptedPacketHandler.hpp"
#include "../common/PacketHandlerFactory.hpp"
#include "../common/InlinePacketHandler.hpp"
#include "../server/Server.hpp"
#include "../utils/Random.hpp"

namespace pkt = senc::pkt;
using senc::server::managers::DecryptionsManager;
using senc::server::managers::UpdateManager;
using senc::server::ShortTermServerStorage;
using senc::server::IServerStorage;
using senc::PacketHandlerImplFactory;
using senc::EncryptedPacketHandler;
using senc::PacketHandlerFactory;
using senc::InlinePacketHandler;
using senc::server::IServer;
using senc::server::Server;
using senc::DecryptionPart;
using senc::PrivKeyShardID;
using senc::PacketHandler;
using senc::utils::Random;
using senc::utils::Buffer;
using senc::PrivKeyShard;
using senc::OperationID;
using senc::utils::Port;
using senc::utils::IPv4;
using senc::utils::IPv6;
using senc::Schema;

using senc::member_count_t;

using senc::OWNER_LAYER;
using senc::REG_LAYER;

using senc::utils::views::enumerate;
using senc::utils::views::join;
using senc::utils::views::zip;

// factory function types for creating member implementations
using StorageFactory = std::function<std::unique_ptr<IServerStorage>()>;
using ServerFactory = std::function<std::unique_ptr<IServer>(
	Schema&, IServerStorage&, PacketHandlerFactory&,
	UpdateManager&, DecryptionsManager&
)>;

using ClientSockPtr = std::unique_ptr<senc::utils::Socket>;
using ClientFactory = std::function<ClientSockPtr(Port)>;

struct CycleParams
{
	member_count_t owners;
	member_count_t regMembers;
	member_count_t nonMembers;
	member_count_t ownersThreshold;
	member_count_t regMembersThreshold;
	std::size_t msgSize;
	int rounds;
};

struct ServerTestParams
{
	ClientFactory clientFactory;
	ServerFactory serverFactory;
	StorageFactory storageFactory;
	std::function<std::unique_ptr<PacketHandlerFactory>()> getPacketHandlerFactory;
};

class ServerTestBase : public testing::Test
{
protected:
	Port port;
	Schema schema;
	std::unique_ptr<PacketHandlerFactory> packetHandlerFactory;
	UpdateManager updateManager;
	DecryptionsManager decryptionsManager;
	std::unique_ptr<IServerStorage> storage;
	std::unique_ptr<IServer> server;

	virtual const ServerTestParams& get_server_test_params() = 0;

	void SetUp() override
	{
		const auto& params = get_server_test_params();
		packetHandlerFactory = params.getPacketHandlerFactory();
		storage = params.storageFactory();
		server = params.serverFactory(
			schema,
			*storage,
			*packetHandlerFactory,
			updateManager,
			decryptionsManager
		);
		server->start();
		port = server->port();
	}

	void TearDown() override
	{
		server->stop();
		storage.reset();
		server.reset();
	}

	std::pair<ClientSockPtr, std::unique_ptr<PacketHandler>> new_client()
	{
		const auto& params = get_server_test_params();
		auto client = params.clientFactory(port);
		auto packetHandler = packetHandlerFactory->new_client_packet_handler(*client);
		return std::make_pair(std::move(client), std::move(packetHandler));
	}

	template <typename Response>
	auto post(PacketHandler& packetHandler, const auto& request) const
	{
		packetHandler.send_request(request);
		return packetHandler.recv_response<Response>();
	}
};

class ServerTest : public ServerTestBase, public testing::WithParamInterface<ServerTestParams>
{
protected:
	const ServerTestParams& get_server_test_params() override
	{
		return GetParam();
	}
};

class MultiCycleServerTest : public ServerTestBase,
	public testing::WithParamInterface<std::tuple<ServerTestParams, CycleParams>>
{
protected:
	const ServerTestParams& get_server_test_params() override
	{
		return std::get<0>(GetParam());
	}

	const CycleParams& get_cycle_params()
	{
		return std::get<1>(GetParam());
	}
};

TEST_P(ServerTest, LogoutWithoutLogin)
{
	auto [client, clientPacketHandler] = new_client();
	auto lo = post<pkt::LogoutResponse>(*clientPacketHandler, pkt::LogoutRequest{});
	EXPECT_TRUE(lo.has_value());
}

TEST_P(ServerTest, SignupAndLogin)
{
	auto [avi, aviPacketHandler] = new_client();
	auto [batya, batyaPacketHandler] = new_client();

	// signup
	auto su1 = post<pkt::SignupResponse>(*aviPacketHandler, pkt::SignupRequest{ "avi", "pass123" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(*batyaPacketHandler, pkt::SignupRequest{ "batya", "pass123" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);

	// check users exist
	EXPECT_TRUE(storage->user_exists("avi"));
	EXPECT_TRUE(storage->user_exists("batya"));

	// logout
	auto lo1 = post<pkt::LogoutResponse>(*aviPacketHandler, pkt::LogoutRequest{});
	EXPECT_TRUE(lo1.has_value());
	auto lo2 = post<pkt::LogoutResponse>(*batyaPacketHandler, pkt::LogoutRequest{});
	EXPECT_TRUE(lo2.has_value());

	// log back in (wrong password)
	std::tie(avi, aviPacketHandler) = new_client();
	std::tie(batya, batyaPacketHandler) = new_client();
	auto li1 = post<pkt::LoginResponse>(*aviPacketHandler, pkt::LoginRequest{ "avi", "AAAA" });
	EXPECT_TRUE(li1.has_value() && li1->status == pkt::LoginResponse::Status::BadLogin);

	// log back in (correct password)
	li1 = post<pkt::LoginResponse>(*aviPacketHandler, pkt::LoginRequest{ "avi" , "pass123" });
	EXPECT_TRUE(li1.has_value() && li1->status == pkt::LoginResponse::Status::Success);
	auto li2 = post<pkt::LoginResponse>(*batyaPacketHandler, pkt::LoginRequest{ "batya" , "pass123" });
	EXPECT_TRUE(li2.has_value() && li2->status == pkt::LoginResponse::Status::Success);

	// logout
	lo1 = post<pkt::LogoutResponse>(*aviPacketHandler, pkt::LogoutRequest{});
	EXPECT_TRUE(lo1.has_value());
	lo2 = post<pkt::LogoutResponse>(*batyaPacketHandler, pkt::LogoutRequest{});
	EXPECT_TRUE(lo2.has_value());
}

TEST_P(ServerTest, MakeSetGetMembers)
{
	auto [client1, client1PacketHandler] = new_client();
	auto [client2, client2PacketHandler] = new_client();
	auto [client3, client3PacketHandler] = new_client();

	// signup
	const auto u1 = "avi";
	auto su1 = post<pkt::SignupResponse>(*client1PacketHandler, pkt::SignupRequest{ u1, "pass123" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	const auto u2 = "batya";
	auto su2 = post<pkt::SignupResponse>(*client2PacketHandler, pkt::SignupRequest{ u2, "pass123" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	const auto u3 = "gal";
	auto su3 = post<pkt::SignupResponse>(*client3PacketHandler, pkt::SignupRequest{ u3, "pass123" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set
	auto ms = post<pkt::MakeUserSetResponse>(*client1PacketHandler, pkt::MakeUserSetRequest{
		.reg_members = { u2 },
		.owners = { u3 },
		.reg_members_threshold = 1,
		.owners_threshold = 1
	});
	EXPECT_TRUE(ms.has_value());
	const auto& usersetID = ms->user_set_id;

	// for each owner (u1 and u3)
	for (auto& clientPacketHandler : { std::ref(*client1PacketHandler), std::ref(*client3PacketHandler) })
	{
		// get sets
		auto gs = post<pkt::GetUserSetsResponse>(clientPacketHandler, pkt::GetUserSetsRequest{});
		EXPECT_TRUE(gs.has_value());

		// check that `usersetID` is in sets
		EXPECT_CONTAINS(gs->user_sets_ids, usersetID);

		// get members
		auto gm = post<pkt::GetMembersResponse>(clientPacketHandler, pkt::GetMembersRequest{ usersetID });
		EXPECT_TRUE(gm.has_value());

		// check that u1 and u3 are owners
		for (const auto& owner : { u1, u3 })
			EXPECT_CONTAINS(gm->owners, owner);

		// check that u2 is a (regular) member
		EXPECT_CONTAINS(gm->reg_members, u2);
	}

	// logout
	for (auto& clientPacketHandler : {
		std::ref(*client1PacketHandler),
		std::ref(*client2PacketHandler),
		std::ref(*client3PacketHandler) })
	{
		auto lo = post<pkt::LogoutResponse>(clientPacketHandler, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, MakeSetCheckKey)
{
	auto [client1, client1PacketHandler] = new_client();
	auto [client2, client2PacketHandler] = new_client();
	auto [client3, client3PacketHandler] = new_client();

	// signup
	const auto u1 = "avi";
	auto su1 = post<pkt::SignupResponse>(*client1PacketHandler, pkt::SignupRequest{ u1, "pass123" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	const auto u2 = "batya";
	auto su2 = post<pkt::SignupResponse>(*client2PacketHandler, pkt::SignupRequest{ u2, "pass123" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	const auto u3 = "gal";
	auto su3 = post<pkt::SignupResponse>(*client3PacketHandler, pkt::SignupRequest{ u3, "pass123" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set
	auto ms = post<pkt::MakeUserSetResponse>(*client1PacketHandler, pkt::MakeUserSetRequest{
		.reg_members = { u2, u3 },
		.owners = { },
		.reg_members_threshold = 2,
		.owners_threshold = 0
	});
	EXPECT_TRUE(ms.has_value());
	const auto& usersetID = ms->user_set_id;
	const auto& pubRegLayerKey = ms->reg_layer_pub_key;
	const auto& pubOwnerLayerKey = ms->owner_layer_pub_key;
	auto& ownerRegLayerShard = ms->reg_layer_priv_key_shard;
	auto& ownerOwnerLayerShard = ms->owner_layer_priv_key_shard;

	EXPECT_NE(ownerRegLayerShard.first, 0);
	EXPECT_NE(ownerOwnerLayerShard.first, 0);

	std::vector<PrivKeyShardID> regLayerShardsIDs, ownerLayerShardsIDs;
	std::vector<PrivKeyShard> regLayerShards, ownerLayerShards;
	regLayerShardsIDs.push_back(ownerRegLayerShard.first);
	regLayerShards.emplace_back(std::move(ownerRegLayerShard));
	ownerLayerShardsIDs.push_back(ownerOwnerLayerShard.first);
	ownerLayerShards.emplace_back(std::move(ownerOwnerLayerShard));

	for (auto& clientPacketHandler : { std::ref(*client2PacketHandler), std::ref(*client3PacketHandler) })
	{
		// get userset update, check same userset ID, get shard
		auto up = post<pkt::UpdateResponse>(clientPacketHandler, pkt::UpdateRequest{});
		EXPECT_TRUE(up.has_value());
		EXPECT_EQ(up->added_as_reg_member.size(), 1);
		EXPECT_EQ(up->added_as_reg_member.front().user_set_id, usersetID);
		auto& shard = up->added_as_reg_member.front().reg_layer_priv_key_shard;

		// check unique ID and shard
		EXPECT_EQ(std::find(regLayerShardsIDs.begin(), regLayerShardsIDs.end(), shard.first), regLayerShardsIDs.end());
		EXPECT_EQ(std::find(regLayerShards.begin(), regLayerShards.end(), shard), regLayerShards.end());

		// check non-zero ID
		EXPECT_NE(shard.first, 0);

		// store shard
		regLayerShardsIDs.push_back(shard.first);
		regLayerShards.push_back(std::move(shard));
	}

	// try to decrypt some message using these shards
	const std::string str = "Hello There";
	const Buffer msg(str.begin(), str.end());
	Schema schema;
	const auto ciphertext = schema.encrypt(msg, pubRegLayerKey, pubOwnerLayerKey);

	std::vector<DecryptionPart> regLayerParts, ownerLayerParts;
	for (const auto& regLayerShard : regLayerShards)
		regLayerParts.emplace_back(
			senc::Shamir::decrypt_get_2l<REG_LAYER>(ciphertext, regLayerShard, regLayerShardsIDs)
		);
	for (const auto& ownerLayerShard : ownerLayerShards)
		ownerLayerParts.emplace_back(
			senc::Shamir::decrypt_get_2l<OWNER_LAYER>(ciphertext, ownerLayerShard, ownerLayerShardsIDs)
		);
	const auto decrypted = senc::Shamir::decrypt_join_2l(ciphertext, regLayerParts, ownerLayerParts);
	EXPECT_EQ(decrypted, msg);

	// logout
	for (auto& clientPacketHandler : {
		std::ref(*client1PacketHandler),
		std::ref(*client2PacketHandler),
		std::ref(*client3PacketHandler) })
	{
		auto lo = post<pkt::LogoutResponse>(clientPacketHandler, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, EmptyUpdateCycle)
{
	auto [client, clientPacketHandler] = new_client();

	// signup
	auto su = post<pkt::SignupResponse>(*clientPacketHandler, pkt::SignupRequest{ "avi", "pass123" });
	EXPECT_TRUE(su.has_value() && su->status == pkt::SignupResponse::Status::Success);

	// update
	auto up = post<pkt::UpdateResponse>(*clientPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up.has_value());

	EXPECT_TRUE(up->added_as_reg_member.empty());
	EXPECT_TRUE(up->added_as_owner.empty());
	EXPECT_TRUE(up->to_decrypt.empty());
	EXPECT_TRUE(up->finished_decryptions.empty());

	// logout
	auto lo = post<pkt::LogoutResponse>(*clientPacketHandler, pkt::LogoutRequest{});
	EXPECT_TRUE(lo.has_value());
}

TEST_P(ServerTest, DecryptFlowSimple)
{
	auto [owner, ownerPacketHandler] = new_client();
	auto [member, memberPacketHandler] = new_client();

	// signup
	auto su1 = post<pkt::SignupResponse>(*ownerPacketHandler, pkt::SignupRequest{ "owner", "pass123" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(*memberPacketHandler, pkt::SignupRequest{ "member", "pass123" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=1
	auto ms = post<pkt::MakeUserSetResponse>(*ownerPacketHandler, pkt::MakeUserSetRequest{
		.reg_members = { "member" },
		.owners = { },
		.reg_members_threshold = 1,
		.owners_threshold = 0
	});
	EXPECT_TRUE(ms.has_value());
	const auto& ownerUsersetID = ms->user_set_id;
	const auto& ownerPubRegLayerKey = ms->reg_layer_pub_key;
	const auto& ownerPubOwnerLayerKey = ms->owner_layer_pub_key;
	const auto& ownerRegLayerShard = ms->reg_layer_priv_key_shard;
	const auto& ownerOwnerLayerShard = ms->owner_layer_priv_key_shard;

	// encrypt a message
	Schema schema;
	const std::string msgStr = "Hello There";
	const Buffer msg(msgStr.begin(), msgStr.end());
	auto ownerCiphertext = schema.encrypt(msg, ownerPubRegLayerKey, ownerPubOwnerLayerKey);

	// 1) owner starts decryption
	auto dc = post<pkt::DecryptResponse>(*ownerPacketHandler, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) member runs update to get decryption lookup request
	auto up1 = post<pkt::UpdateResponse>(*memberPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up1.has_value());
	const auto& memberSetsAddedTo = up1->added_as_reg_member;
	const auto& memberOnLookup = up1->on_lookup;

	//    member was added to one set, check same as owner's
	EXPECT_EQ(memberSetsAddedTo.size(), 1);
	EXPECT_EQ(memberSetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(memberSetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(memberSetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& memberShard = memberSetsAddedTo.front().reg_layer_priv_key_shard;

	//    member has one operation to participate in, check same as owner's
	EXPECT_EQ(memberOnLookup.size(), 1);
	EXPECT_EQ(memberOnLookup.front(), ownerOpid);

	// 3) member tells server that they're willing to participate in operation
	auto dp = post<pkt::DecryptParticipateResponse>(*memberPacketHandler, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp.has_value() && dp->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

	// 4) member runs update to get decryption request
	auto up2 = post<pkt::UpdateResponse>(*memberPacketHandler, pkt::UpdateRequest{});
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
	auto memberPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		memberCiphertext,
		memberShard,
		memberShardsIDs
	);
	// (member knows it's not an owner, so layer 1)

	// 6) member sends decryption part back
	auto sp = post<pkt::SendDecryptionPartResponse>(*memberPacketHandler, pkt::SendDecryptionPartRequest{
		.op_id = memberOpid,
		.decryption_part = memberPart
	});
	EXPECT_TRUE(sp.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(*ownerPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up3.has_value());

	//    member has one finished decrytion, check same as submitted
	auto& finished = up3->finished_decryptions;
	EXPECT_EQ(finished.size(), 1);
	EXPECT_EQ(finished.front().op_id, ownerOpid);

	auto& finishedRegLayerShardsIDs = finished.front().reg_layer_shards_ids;
	auto& finishedOwnerLayerShardsIDs = finished.front().owner_layer_shards_ids;
	auto& finishedRegLayerParts = finished.front().reg_layer_parts;
	auto& finishedOwnerLayerParts = finished.front().owner_layer_parts;
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), finishedRegLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), finishedOwnerLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), 2); // two shards, owner+member
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), 1); // owner shard only

	// 8) owner computes their own decryption parts
	auto ownerRegLayerPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		ownerCiphertext,
		ownerRegLayerShard,
		finishedRegLayerShardsIDs
	);
	auto ownerOwnerLayerPart = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
		ownerCiphertext,
		ownerOwnerLayerShard,
		finishedOwnerLayerShardsIDs
	);

	// 9) owner combines their parts with received parts and decrypts fully
	std::vector<DecryptionPart> regLayerParts = finishedRegLayerParts;
	regLayerParts.push_back(ownerRegLayerPart);
	std::vector<DecryptionPart> ownerLayerParts = finishedOwnerLayerParts;
	ownerLayerParts.push_back(ownerOwnerLayerPart);
	auto decrypted = senc::Shamir::decrypt_join_2l(
		ownerCiphertext, regLayerParts, ownerLayerParts
	);
	EXPECT_EQ(decrypted, msg);

	// logout
	for (auto& clientPacketHandler : { std::ref(*ownerPacketHandler), std::ref(*memberPacketHandler) })
	{
		auto lo = post<pkt::LogoutResponse>(clientPacketHandler, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, DecryptFlowTwoMembers)
{
	auto [owner, ownerPacketHandler] = new_client();
	auto [member, memberPacketHandler] = new_client();
	auto [member2, member2PacketHandler] = new_client();

	// signup
	auto su1 = post<pkt::SignupResponse>(*ownerPacketHandler, pkt::SignupRequest{ "owner", "pass123" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(*memberPacketHandler, pkt::SignupRequest{ "member", "pass123" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	auto su3 = post<pkt::SignupResponse>(*member2PacketHandler, pkt::SignupRequest{ "member2", "pass123" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=1
	auto ms = post<pkt::MakeUserSetResponse>(*ownerPacketHandler, pkt::MakeUserSetRequest{
		.reg_members = { "member", "member2" },
		.owners = { },
		.reg_members_threshold = 2,
		.owners_threshold = 0
	});
	EXPECT_TRUE(ms.has_value());
	const auto& ownerUsersetID = ms->user_set_id;
	const auto& ownerPubRegLayerKey = ms->reg_layer_pub_key;
	const auto& ownerPubOwnerLayerKey = ms->owner_layer_pub_key;
	const auto& ownerRegLayerShard = ms->reg_layer_priv_key_shard;
	const auto& ownerOwnerLayerShard = ms->owner_layer_priv_key_shard;

	// encrypt a message
	Schema schema;
	const std::string msgStr = "Hello There";
	const Buffer msg(msgStr.begin(), msgStr.end());
	auto ownerCiphertext = schema.encrypt(msg, ownerPubRegLayerKey, ownerPubOwnerLayerKey);

	// 1) owner starts decryption
	auto dc = post<pkt::DecryptResponse>(*ownerPacketHandler, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) member runs update to get decryption lookup request
	auto up1a = post<pkt::UpdateResponse>(*memberPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up1a.has_value());
	const auto& memberSetsAddedTo = up1a->added_as_reg_member;
	const auto& memberOnLookup = up1a->on_lookup;

	auto up1b = post<pkt::UpdateResponse>(*member2PacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up1b.has_value());
	const auto& member2SetsAddedTo = up1b->added_as_reg_member;
	const auto& member2OnLookup = up1b->on_lookup;

	//    members were added to one set, check same as owner's
	EXPECT_EQ(memberSetsAddedTo.size(), 1);
	EXPECT_EQ(memberSetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(memberSetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(memberSetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& memberShard = memberSetsAddedTo.front().reg_layer_priv_key_shard;

	EXPECT_EQ(member2SetsAddedTo.size(), 1);
	EXPECT_EQ(member2SetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(member2SetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(member2SetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& member2Shard = member2SetsAddedTo.front().reg_layer_priv_key_shard;

	// check different shard IDs
	EXPECT_NE(ownerOwnerLayerShard.first, memberShard.first);
	EXPECT_NE(ownerOwnerLayerShard.first, member2Shard.first);
	EXPECT_NE(memberShard.first, member2Shard.first);

	//    members have one operation to participate in, check same as owner's
	EXPECT_EQ(memberOnLookup.size(), 1);
	EXPECT_EQ(memberOnLookup.front(), ownerOpid);

	EXPECT_EQ(member2OnLookup.size(), 1);
	EXPECT_EQ(member2OnLookup.front(), ownerOpid);

	// 3) members tell server that they're willing to participate in operation
	auto dp1 = post<pkt::DecryptParticipateResponse>(*memberPacketHandler, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp1.has_value() && dp1->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

	auto dp2 = post<pkt::DecryptParticipateResponse>(*member2PacketHandler, pkt::DecryptParticipateRequest{
		member2OnLookup.front()
	});
	EXPECT_TRUE(dp2.has_value() && dp2->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

	// 4) members run update to get decryption request
	auto up2a = post<pkt::UpdateResponse>(*memberPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up2a.has_value());
	const auto& memberToDecrypt = up2a->to_decrypt;

	auto up2b = post<pkt::UpdateResponse>(*member2PacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up2b.has_value());
	const auto& member2ToDecrypt = up2b->to_decrypt;

	//    members have one part to decrypt, check same operation as owner
	EXPECT_EQ(memberToDecrypt.size(), 1);
	const auto& memberOpid = memberToDecrypt.front().op_id;
	const auto& memberCiphertext = memberToDecrypt.front().ciphertext;
	const auto& memberShardsIDs = memberToDecrypt.front().shards_ids;
	EXPECT_EQ(memberOpid, ownerOpid);
	EXPECT_EQ(memberCiphertext, ownerCiphertext);

	EXPECT_EQ(member2ToDecrypt.size(), 1);
	const auto& member2Opid = member2ToDecrypt.front().op_id;
	const auto& member2Ciphertext = member2ToDecrypt.front().ciphertext;
	const auto& member2ShardsIDs = member2ToDecrypt.front().shards_ids;
	EXPECT_EQ(member2Opid, ownerOpid);
	EXPECT_EQ(member2Ciphertext, ownerCiphertext);

	// 5) members compute decryption part locally
	// (members know they're not owners, so layer 1)
	auto memberPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		memberCiphertext,
		memberShard,
		memberShardsIDs
	);
	auto member2Part = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		member2Ciphertext,
		member2Shard,
		member2ShardsIDs
	);

	// 6) members send decryption part back
	auto sp1 = post<pkt::SendDecryptionPartResponse>(*memberPacketHandler, pkt::SendDecryptionPartRequest{
		.op_id = memberOpid,
		.decryption_part = memberPart
	});
	EXPECT_TRUE(sp1.has_value());

	auto sp2 = post<pkt::SendDecryptionPartResponse>(*member2PacketHandler, pkt::SendDecryptionPartRequest{
		.op_id = member2Opid,
		.decryption_part = member2Part
	});
	EXPECT_TRUE(sp2.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(*ownerPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up3.has_value());

	//    owner has one finished decrytion, check same as submitted
	auto& finished = up3->finished_decryptions;
	EXPECT_EQ(finished.size(), 1);
	EXPECT_EQ(finished.front().op_id, ownerOpid);

	auto& finishedRegLayerShardsIDs = finished.front().reg_layer_shards_ids;
	auto& finishedOwnerLayerShardsIDs = finished.front().owner_layer_shards_ids;
	auto& finishedRegLayerParts = finished.front().reg_layer_parts;
	auto& finishedOwnerLayerParts = finished.front().owner_layer_parts;
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), finishedRegLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), finishedOwnerLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), 3); // three shards, owner + two members
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), 1); // owner shard only

	// 8) owner computes their own decryption parts
	auto ownerRegLayerPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		ownerCiphertext,
		ownerRegLayerShard,
		finishedRegLayerShardsIDs
	);
	auto ownerOwnerLayerPart = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
		ownerCiphertext,
		ownerOwnerLayerShard,
		finishedOwnerLayerShardsIDs
	);

	// 9) owner combines their parts with received parts and decrypts fully
	std::vector<DecryptionPart> regLayerParts = finishedRegLayerParts;
	regLayerParts.push_back(ownerRegLayerPart);
	std::vector<DecryptionPart> ownerLayerParts = finishedOwnerLayerParts;
	ownerLayerParts.push_back(ownerOwnerLayerPart);
	EXPECT_GT(regLayerParts.size(), 2); // regMembersThreahold=2
	EXPECT_GT(ownerLayerParts.size(), 0); // ownersThreahold=0
	auto decrypted = senc::Shamir::decrypt_join_2l(
		ownerCiphertext, regLayerParts, ownerLayerParts
	);
	EXPECT_EQ(decrypted, msg);

	// logout
	for (auto& clientPacketHandler : {
		std::ref(*ownerPacketHandler),
		std::ref(*memberPacketHandler),
		std::ref(*member2PacketHandler) })
	{
		auto lo = post<pkt::LogoutResponse>(clientPacketHandler, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, DecryptFlowExtraMember)
{
	auto [owner, ownerPacketHandler] = new_client();
	auto [member, memberPacketHandler] = new_client();
	auto [extra, extraPacketHandler] = new_client();

	// signup
	auto su1 = post<pkt::SignupResponse>(*ownerPacketHandler, pkt::SignupRequest{ "owner", "pass123" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(*memberPacketHandler, pkt::SignupRequest{ "member", "pass123" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	auto su3 = post<pkt::SignupResponse>(*extraPacketHandler, pkt::SignupRequest{ "extra", "pass123" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=1
	auto ms = post<pkt::MakeUserSetResponse>(*ownerPacketHandler, pkt::MakeUserSetRequest{
		.reg_members = { "member", "extra" },
		.owners = { },
		.reg_members_threshold = 1,
		.owners_threshold = 0
	});
	EXPECT_TRUE(ms.has_value());
	const auto& ownerUsersetID = ms->user_set_id;
	const auto& ownerPubRegLayerKey = ms->reg_layer_pub_key;
	const auto& ownerPubOwnerLayerKey = ms->owner_layer_pub_key;
	const auto& ownerRegLayerShard = ms->reg_layer_priv_key_shard;
	const auto& ownerOwnerLayerShard = ms->owner_layer_priv_key_shard;

	// encrypt a message
	Schema schema;
	const std::string msgStr = "Hello There";
	const Buffer msg(msgStr.begin(), msgStr.end());
	auto ownerCiphertext = schema.encrypt(msg, ownerPubRegLayerKey, ownerPubOwnerLayerKey);

	// 1) owner starts decryption
	auto dc = post<pkt::DecryptResponse>(*ownerPacketHandler, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) member runs update to get decryption lookup request
	auto up1 = post<pkt::UpdateResponse>(*memberPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up1.has_value());
	const auto& memberSetsAddedTo = up1->added_as_reg_member;
	const auto& memberOnLookup = up1->on_lookup;

	//    member was added to one set, check same as owner's
	EXPECT_EQ(memberSetsAddedTo.size(), 1);
	EXPECT_EQ(memberSetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(memberSetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(memberSetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& memberShard = memberSetsAddedTo.front().reg_layer_priv_key_shard;

	//    member has one operation to participate in, check same as owner's
	EXPECT_EQ(memberOnLookup.size(), 1);
	EXPECT_EQ(memberOnLookup.front(), ownerOpid);

	// (extra does same, but will not participate in decryption)
	auto upe = post<pkt::UpdateResponse>(*extraPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(upe.has_value());
	const auto& extraSetsAddedTo = upe->added_as_reg_member;
	EXPECT_EQ(extraSetsAddedTo.size(), 1);
	EXPECT_EQ(extraSetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(extraSetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(extraSetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& extraShard = extraSetsAddedTo.front().reg_layer_priv_key_shard;
	(void)extraShard; // for debugging purposes

	// 3) member tells server that they're willing to participate in operation
	auto dp = post<pkt::DecryptParticipateResponse>(*memberPacketHandler, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp.has_value() && dp->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

	// 4) member runs update to get decryption request
	auto up2 = post<pkt::UpdateResponse>(*memberPacketHandler, pkt::UpdateRequest{});
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
	auto memberPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		memberCiphertext,
		memberShard,
		memberShardsIDs
	);
	// (member knows it's not an owner, so layer 1)

	// 6) member sends decryption part back
	auto sp = post<pkt::SendDecryptionPartResponse>(*memberPacketHandler, pkt::SendDecryptionPartRequest{
		.op_id = memberOpid,
		.decryption_part = memberPart
	});
	EXPECT_TRUE(sp.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(*ownerPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up3.has_value());

	//    member has one finished decrytion, check same as submitted
	auto& finished = up3->finished_decryptions;
	EXPECT_EQ(finished.size(), 1);
	EXPECT_EQ(finished.front().op_id, ownerOpid);

	auto& finishedRegLayerShardsIDs = finished.front().reg_layer_shards_ids;
	auto& finishedOwnerLayerShardsIDs = finished.front().owner_layer_shards_ids;
	auto& finishedRegLayerParts = finished.front().reg_layer_parts;
	auto& finishedOwnerLayerParts = finished.front().owner_layer_parts;
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), finishedRegLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), finishedOwnerLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), 2); // two shards, owner+member
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), 1); // owner shard only

	// 8) owner computes their own decryption parts
	auto ownerRegLayerPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		ownerCiphertext,
		ownerRegLayerShard,
		finishedRegLayerShardsIDs
	);
	auto ownerOwnerLayerPart = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
		ownerCiphertext,
		ownerOwnerLayerShard,
		finishedOwnerLayerShardsIDs
	);

	// 9) owner combines their parts with received parts and decrypts fully
	std::vector<DecryptionPart> regLayerParts = finishedRegLayerParts;
	regLayerParts.push_back(ownerRegLayerPart);
	std::vector<DecryptionPart> ownerLayerParts = finishedOwnerLayerParts;
	ownerLayerParts.push_back(ownerOwnerLayerPart);
	auto decrypted = senc::Shamir::decrypt_join_2l(
		ownerCiphertext, regLayerParts, ownerLayerParts
	);
	EXPECT_EQ(decrypted, msg);

	// logout
	for (auto& clientPacketHandler : {
		std::ref(*ownerPacketHandler),
		std::ref(*memberPacketHandler),
		std::ref(*extraPacketHandler) })
	{
		auto lo = post<pkt::LogoutResponse>(clientPacketHandler, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, DecryptFlow2L)
{
	auto [owner, ownerPacketHandler] = new_client();
	auto [member, memberPacketHandler] = new_client();
	auto [owner2, owner2PacketHandler] = new_client();

	// signup
	auto su1 = post<pkt::SignupResponse>(*ownerPacketHandler, pkt::SignupRequest{ "owner", "pass123" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(*memberPacketHandler, pkt::SignupRequest{ "member", "pass123" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	auto su3 = post<pkt::SignupResponse>(*owner2PacketHandler, pkt::SignupRequest{ "owner2", "pass123" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=1
	auto ms = post<pkt::MakeUserSetResponse>(*ownerPacketHandler, pkt::MakeUserSetRequest{
		.reg_members = { "member" },
		.owners = { "owner2" },
		.reg_members_threshold = 1,
		.owners_threshold = 1
	});
	EXPECT_TRUE(ms.has_value());
	const auto& ownerUsersetID = ms->user_set_id;
	const auto& ownerPubRegLayerKey = ms->reg_layer_pub_key;
	const auto& ownerPubOwnerLayerKey = ms->owner_layer_pub_key;
	const auto& ownerRegLayerShard = ms->reg_layer_priv_key_shard;
	const auto& ownerOwnerLayerShard = ms->owner_layer_priv_key_shard;

	// encrypt a message
	Schema schema;
	const std::string msgStr = "Hello There";
	const Buffer msg(msgStr.begin(), msgStr.end());
	auto ownerCiphertext = schema.encrypt(msg, ownerPubRegLayerKey, ownerPubOwnerLayerKey);

	// 1) owner starts decryption
	auto dc = post<pkt::DecryptResponse>(*ownerPacketHandler, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) members run update to get decryption lookup request
	auto up1 = post<pkt::UpdateResponse>(*memberPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up1.has_value());
	const auto& memberSetsAddedTo = up1->added_as_reg_member;
	const auto& memberOnLookup = up1->on_lookup;

	auto up1b = post<pkt::UpdateResponse>(*owner2PacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up1b.has_value());
	const auto& owner2SetsAddedTo = up1b->added_as_owner;
	const auto& owner2OnLookup = up1b->on_lookup;

	//    members were added to one set, check same as owner's
	EXPECT_EQ(memberSetsAddedTo.size(), 1);
	EXPECT_EQ(memberSetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(memberSetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(memberSetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& memberShard = memberSetsAddedTo.front().reg_layer_priv_key_shard;

	EXPECT_EQ(owner2SetsAddedTo.size(), 1);
	EXPECT_EQ(owner2SetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(owner2SetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(owner2SetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& owner2Shard = owner2SetsAddedTo.front().owner_layer_priv_key_shard;

	//    members have one operation to participate in, check same as owner's
	EXPECT_EQ(memberOnLookup.size(), 1);
	EXPECT_EQ(memberOnLookup.front(), ownerOpid);

	EXPECT_EQ(owner2OnLookup.size(), 1);
	EXPECT_EQ(owner2OnLookup.front(), ownerOpid);

	// 3) members tell server that they're willing to participate in operation
	auto dp = post<pkt::DecryptParticipateResponse>(*memberPacketHandler, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp.has_value() && dp->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

	auto dp2 = post<pkt::DecryptParticipateResponse>(*owner2PacketHandler, pkt::DecryptParticipateRequest{
		owner2OnLookup.front()
	});
	EXPECT_TRUE(dp2.has_value() && dp2->status == pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart);

	// 4) members run update to get decryption request
	auto up2 = post<pkt::UpdateResponse>(*memberPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up2.has_value());
	const auto& memberToDecrypt = up2->to_decrypt;

	auto up2b = post<pkt::UpdateResponse>(*owner2PacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up2b.has_value());
	const auto& owner2ToDecrypt = up2b->to_decrypt;

	//    members have one part to decrypt, check same operation as owner
	EXPECT_EQ(memberToDecrypt.size(), 1);
	const auto& memberOpid = memberToDecrypt.front().op_id;
	const auto& memberCiphertext = memberToDecrypt.front().ciphertext;
	const auto& memberShardsIDs = memberToDecrypt.front().shards_ids;
	EXPECT_EQ(memberOpid, ownerOpid);
	EXPECT_EQ(memberCiphertext, ownerCiphertext);

	EXPECT_EQ(owner2ToDecrypt.size(), 1);
	const auto& owner2Opid = owner2ToDecrypt.front().op_id;
	const auto& owner2Ciphertext = owner2ToDecrypt.front().ciphertext;
	const auto& owner2ShardsIDs = owner2ToDecrypt.front().shards_ids;
	EXPECT_EQ(owner2Opid, ownerOpid);
	EXPECT_EQ(owner2Ciphertext, ownerCiphertext);

	// 5) members compute decryption part locally
	auto memberPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		memberCiphertext,
		memberShard,
		memberShardsIDs
	);

	auto owner2Part = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
		owner2Ciphertext,
		owner2Shard,
		owner2ShardsIDs
	);

	// 6) members send decryption part back
	auto sp = post<pkt::SendDecryptionPartResponse>(*memberPacketHandler, pkt::SendDecryptionPartRequest{
		.op_id = memberOpid,
		.decryption_part = memberPart
	});
	EXPECT_TRUE(sp.has_value());

	auto sp2 = post<pkt::SendDecryptionPartResponse>(*owner2PacketHandler, pkt::SendDecryptionPartRequest{
		.op_id = owner2Opid,
		.decryption_part = owner2Part
	});
	EXPECT_TRUE(sp2.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(*ownerPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up3.has_value());

	//    owner has one finished decrytion, check same as submitted
	auto& finished = up3->finished_decryptions;
	EXPECT_EQ(finished.size(), 1);
	EXPECT_EQ(finished.front().op_id, ownerOpid);

	auto& finishedRegLayerShardsIDs = finished.front().reg_layer_shards_ids;
	auto& finishedOwnerLayerShardsIDs = finished.front().owner_layer_shards_ids;
	auto& finishedRegLayerParts = finished.front().reg_layer_parts;
	auto& finishedOwnerLayerParts = finished.front().owner_layer_parts;
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), finishedRegLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), finishedOwnerLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), 2); // two shards, owner+member
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), 2); // two shards, owner+owner2

	// 8) owner computes their own decryption parts
	auto ownerRegLayerPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		ownerCiphertext,
		ownerRegLayerShard,
		finishedRegLayerShardsIDs
	);
	auto ownerOwnerLayerPart = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
		ownerCiphertext,
		ownerOwnerLayerShard,
		finishedOwnerLayerShardsIDs
	);

	// 9) owner combines their parts with received parts and decrypts fully
	std::vector<DecryptionPart> regLayerParts = finishedRegLayerParts;
	regLayerParts.push_back(ownerRegLayerPart);
	std::vector<DecryptionPart> ownerLayerParts = finishedOwnerLayerParts;
	ownerLayerParts.push_back(ownerOwnerLayerPart);
	auto decrypted = senc::Shamir::decrypt_join_2l(
		ownerCiphertext, regLayerParts, ownerLayerParts
	);
	EXPECT_EQ(decrypted, msg);

	// logout
	for (auto& clientPacketHandler : {
		std::ref(*ownerPacketHandler),
		std::ref(*memberPacketHandler),
		std::ref(*owner2PacketHandler) })
	{
		auto lo = post<pkt::LogoutResponse>(clientPacketHandler, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, DecryptFlowOwnersOnly)
{
	auto [owner, ownerPacketHandler] = new_client();
	auto [owner2, owner2PacketHandler] = new_client();
	auto [owner3, owner3PacketHandler] = new_client();

	// signup
	auto su1 = post<pkt::SignupResponse>(*ownerPacketHandler, pkt::SignupRequest{ "owner", "pass123" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(*owner2PacketHandler, pkt::SignupRequest{ "owner2", "pass123" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	auto su3 = post<pkt::SignupResponse>(*owner3PacketHandler, pkt::SignupRequest{ "owner3", "pass123" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=2
	auto ms = post<pkt::MakeUserSetResponse>(*ownerPacketHandler, pkt::MakeUserSetRequest{
		.reg_members = { },
		.owners = { "owner2", "owner3" },
		.reg_members_threshold = 0,
		.owners_threshold = 2
	});
	EXPECT_TRUE(ms.has_value());
	const auto& ownerUsersetID = ms->user_set_id;
	const auto& ownerPubRegLayerKey = ms->reg_layer_pub_key;
	const auto& ownerPubOwnerLayerKey = ms->owner_layer_pub_key;
	const auto& ownerRegLayerShard = ms->reg_layer_priv_key_shard;
	const auto& ownerOwnerLayerShard = ms->owner_layer_priv_key_shard;

	// encrypt a message
	Schema schema;
	const std::string msgStr = "Hello There";
	const Buffer msg(msgStr.begin(), msgStr.end());
	auto ownerCiphertext = schema.encrypt(msg, ownerPubRegLayerKey, ownerPubOwnerLayerKey);

	// 1) owner starts decryption
	auto dc = post<pkt::DecryptResponse>(*ownerPacketHandler, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) members run update to get decryption lookup request
	auto up1a = post<pkt::UpdateResponse>(*owner2PacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up1a.has_value());
	const auto& owner2SetsAddedTo = up1a->added_as_owner;
	const auto& owner2OnLookup = up1a->on_lookup;

	auto up1b = post<pkt::UpdateResponse>(*owner3PacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up1b.has_value());
	const auto& owner3SetsAddedTo = up1b->added_as_owner;
	const auto& owner3OnLookup = up1b->on_lookup;

	//    members were added to one set, check same as owner's
	EXPECT_EQ(owner2SetsAddedTo.size(), 1);
	EXPECT_EQ(owner2SetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(owner2SetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(owner2SetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& owner2Shard = owner2SetsAddedTo.front().owner_layer_priv_key_shard;

	EXPECT_EQ(owner3SetsAddedTo.size(), 1);
	EXPECT_EQ(owner3SetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(owner3SetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(owner3SetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& owner3Shard = owner3SetsAddedTo.front().owner_layer_priv_key_shard;

	//    members have one operation to participate in, check same as owner's
	EXPECT_EQ(owner2OnLookup.size(), 1);
	EXPECT_EQ(owner2OnLookup.front(), ownerOpid);

	EXPECT_EQ(owner3OnLookup.size(), 1);
	EXPECT_EQ(owner3OnLookup.front(), ownerOpid);

	// 3) members tell server that they're willing to participate in operation
	auto dp1 = post<pkt::DecryptParticipateResponse>(*owner2PacketHandler, pkt::DecryptParticipateRequest{
		owner2OnLookup.front()
	});
	EXPECT_TRUE(dp1.has_value() && dp1->status == pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart);

	auto dp2 = post<pkt::DecryptParticipateResponse>(*owner3PacketHandler, pkt::DecryptParticipateRequest{
		owner3OnLookup.front()
	});
	EXPECT_TRUE(dp2.has_value() && dp2->status == pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart);

	// 4) members run update to get decryption request
	auto up2a = post<pkt::UpdateResponse>(*owner2PacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up2a.has_value());
	const auto& owner2ToDecrypt = up2a->to_decrypt;

	auto up2b = post<pkt::UpdateResponse>(*owner3PacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up2b.has_value());
	const auto& owner3ToDecrypt = up2b->to_decrypt;

	//    members have one part to decrypt, check same operation as owner
	EXPECT_EQ(owner2ToDecrypt.size(), 1);
	const auto& owner2Opid = owner2ToDecrypt.front().op_id;
	const auto& owner2Ciphertext = owner2ToDecrypt.front().ciphertext;
	const auto& owner2ShardsIDs = owner2ToDecrypt.front().shards_ids;
	EXPECT_EQ(owner2Opid, ownerOpid);
	EXPECT_EQ(owner2Ciphertext, ownerCiphertext);

	EXPECT_EQ(owner3ToDecrypt.size(), 1);
	const auto& owner3Opid = owner3ToDecrypt.front().op_id;
	const auto& owner3Ciphertext = owner3ToDecrypt.front().ciphertext;
	const auto& owner3ShardsIDs = owner3ToDecrypt.front().shards_ids;
	EXPECT_EQ(owner3Opid, ownerOpid);
	EXPECT_EQ(owner3Ciphertext, ownerCiphertext);

	// 5) members compute decryption part locally
	auto owner2Part = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
		owner2Ciphertext,
		owner2Shard,
		owner2ShardsIDs
	);
	auto owner3Part = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
		owner3Ciphertext,
		owner3Shard,
		owner3ShardsIDs
	);

	// 6) members send decryption part back
	auto sp1 = post<pkt::SendDecryptionPartResponse>(*owner2PacketHandler, pkt::SendDecryptionPartRequest{
		.op_id = owner2Opid,
		.decryption_part = owner2Part
	});
	EXPECT_TRUE(sp1.has_value());

	auto sp2 = post<pkt::SendDecryptionPartResponse>(*owner3PacketHandler, pkt::SendDecryptionPartRequest{
		.op_id = owner3Opid,
		.decryption_part = owner3Part
	});
	EXPECT_TRUE(sp2.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(*ownerPacketHandler, pkt::UpdateRequest{});
	EXPECT_TRUE(up3.has_value());

	//    owner has one finished decrytion, check same as submitted
	auto& finished = up3->finished_decryptions;
	EXPECT_EQ(finished.size(), 1);
	EXPECT_EQ(finished.front().op_id, ownerOpid);

	auto& finishedRegLayerShardsIDs = finished.front().reg_layer_shards_ids;
	auto& finishedOwnerLayerShardsIDs = finished.front().owner_layer_shards_ids;
	auto& finishedRegLayerParts = finished.front().reg_layer_parts;
	auto& finishedOwnerLayerParts = finished.front().owner_layer_parts;
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), finishedRegLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), finishedOwnerLayerParts.size() + 1); // including owner shard
	EXPECT_EQ(finishedRegLayerShardsIDs.size(), 1); // owner shard only
	EXPECT_EQ(finishedOwnerLayerShardsIDs.size(), 3); // three shards, owner + two more owners

	// 8) owner computes their own decryption parts
	auto ownerRegLayerPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
		ownerCiphertext,
		ownerRegLayerShard,
		finishedRegLayerShardsIDs
	);
	auto ownerOwnerLayerPart = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
		ownerCiphertext,
		ownerOwnerLayerShard,
		finishedOwnerLayerShardsIDs
	);

	// 9) owner combines their parts with received parts and decrypts fully
	std::vector<DecryptionPart> regLayerParts = finishedRegLayerParts;
	regLayerParts.push_back(ownerRegLayerPart);
	std::vector<DecryptionPart> ownerLayerParts = finishedOwnerLayerParts;
	ownerLayerParts.push_back(ownerOwnerLayerPart);
	auto decrypted = senc::Shamir::decrypt_join_2l(
		ownerCiphertext, regLayerParts, ownerLayerParts
	);
	EXPECT_EQ(decrypted, msg);

	// logout
	for (auto& clientPacketHandler : {
		std::ref(*ownerPacketHandler),
		std::ref(*owner2PacketHandler),
		std::ref(*owner3PacketHandler) })
	{
		auto lo = post<pkt::LogoutResponse>(clientPacketHandler, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(MultiCycleServerTest, MultiCycleDecryptFlow2L)
{
	using SockPacketHandlerPair = std::pair<ClientSockPtr, std::unique_ptr<PacketHandler>>;

	auto vecToPacketHandlers = []()
	{
		return std::views::all |
			std::views::transform([](SockPacketHandlerPair& p) -> PacketHandler& { return *p.second; });
	};

	auto makeusers = [this](std::size_t size, const char* prefix)
	{
		std::vector<SockPacketHandlerPair> socketsPacketHandlers;
		std::vector<std::string> usernames;
		for (std::size_t i = 0; i < size; ++i)
		{
			socketsPacketHandlers.emplace_back(new_client());
			usernames.push_back(prefix + std::to_string(i));
		}
		return std::make_tuple(
			std::move(socketsPacketHandlers),
			std::move(usernames)
		);
	};

	const auto& params = get_cycle_params();

	// users:
	// - creator
	// - ownerThreshold additional involved owners
	//   (in total we have +1 with creator, one needs to be the initiator)
	// - regMemberThreshold involved non-owners
	// - remaining (uninvolved) owners and non-owners to fill up params
	// - non-members

	std::vector<std::pair<ClientSockPtr, std::unique_ptr<PacketHandler>>> creatorSocksPacketHandlersVec;
	creatorSocksPacketHandlersVec.emplace_back(new_client());
	std::vector<std::string> creatorUsernamesVec = { "creator" };

	auto creatorPacketHandlers = creatorSocksPacketHandlersVec | vecToPacketHandlers();
	auto creatorUsernames = creatorUsernamesVec | std::views::all;

	auto& creatorPacketHandler = *creatorSocksPacketHandlersVec.front().second;

	auto [nonCreatorOwnerSocksPacketHandlersVec, nonCreatorOwnerUsernamesVec] = makeusers(
		params.owners, "owner"
	);
	auto nonCreatorOwnerPacketHandlers = nonCreatorOwnerSocksPacketHandlersVec | vecToPacketHandlers();
	auto nonCreatorOwnerUsernames = nonCreatorOwnerUsernamesVec | std::views::all;
	auto [regMemberSocksPacketHandlersVec, regMemberUsernamesVec] = makeusers(params.regMembers, "reg");
	auto regMemberPacketHandlers = regMemberSocksPacketHandlersVec | vecToPacketHandlers();
	auto regMemberUsernames = regMemberUsernamesVec | std::views::all;
	auto [nonMemberSocksPacketHandlersVec, nonMemberUsernamesVec] = makeusers(params.nonMembers, "foreign");
	auto nonMemberPacketHandlers = nonMemberSocksPacketHandlersVec | vecToPacketHandlers();
	auto nonMemberUsernames = nonMemberUsernamesVec | std::views::all;

	auto nonCreatorInvolvedOwnerPacketHandlers = nonCreatorOwnerPacketHandlers |
		std::views::take(params.ownersThreshold);
	// auto nonCreatorInvolvedOwnerUsernames = nonCreatorOwnerUsernames |
	// 	std::views::take(params.regMembersThreshold);

	auto involvedOwnerPacketHandlers = join(creatorPacketHandlers, nonCreatorInvolvedOwnerPacketHandlers);
	// auto involvedOwnerUsernames = join(creatorUsernames, nonCreatorInvolvedOwnerUsernames);

	auto uninvolvedOwnerPacketHandlers = nonCreatorOwnerPacketHandlers | std::views::drop(params.ownersThreshold);
	// auto uninvolvedOwnerUsernames = nonCreatorOwnerUsernames | std::views::drop(params.regMembersThreshold);

	auto involvedRegMemberPacketHandlers = regMemberPacketHandlers | std::views::take(params.regMembersThreshold);
	// auto involvedRegMemberUsernames = regMemberUsernames | std::views::take(params.regMembersThreshold);

	auto uninvolvedRegMemberPacketHandlers = regMemberPacketHandlers | std::views::drop(params.regMembersThreshold);
	// auto uninvolvedRegMemberUsernames = regMemberUsernames | std::views::drop(params.regMembersThreshold);

	// auto involvedPacketHandlers = join(involvedOwnerPacketHandlers, involvedRegMemberPacketHandlers);
	auto uninvolvedPacketHandlers = join(uninvolvedOwnerPacketHandlers, uninvolvedRegMemberPacketHandlers);

	auto memberPacketHandlers = join(creatorPacketHandlers, nonCreatorOwnerPacketHandlers, regMemberPacketHandlers);
	auto memberUsernames = join(creatorUsernames, nonCreatorOwnerUsernames, regMemberUsernames);

	auto allPacketHandlers = join(memberPacketHandlers, nonMemberPacketHandlers);
	auto allUsernames = join(memberUsernames, nonMemberUsernames);

	// signup
	for (auto [packetHandler, username] : zip(allPacketHandlers, allUsernames))
	{
		std::optional<pkt::SignupResponse> su = 
			post<pkt::SignupResponse>(packetHandler, pkt::SignupRequest{ username, "pass123" });
		EXPECT_TRUE(su.has_value());
		EXPECT_EQ(su->status, pkt::SignupResponse::Status::Success);
	}

	// vectors to store shards later
	std::vector<PrivKeyShard> regMemberShards;
	std::vector<PrivKeyShardID> regMemberShardsIDs;
	std::vector<PrivKeyShard> ownerRegLayerShards, ownerOwnerLayerShards;
	std::vector<PrivKeyShardID> ownerRegLayerShardsIDs, ownerOwnerLayerShardsIDs;

	// make userset
	auto ms = post<pkt::MakeUserSetResponse>(creatorPacketHandler, pkt::MakeUserSetRequest{
		.reg_members = regMemberUsernamesVec,
		.owners = nonCreatorOwnerUsernamesVec,
		.reg_members_threshold = params.regMembersThreshold,
		.owners_threshold = params.ownersThreshold
	});
	EXPECT_TRUE(ms.has_value());
	const auto& usersetID = ms->user_set_id;
	const auto& pubRegLayerKey = ms->reg_layer_pub_key;
	const auto& pubOwnerLayerKey = ms->owner_layer_pub_key;
	ownerRegLayerShardsIDs.push_back(ms->reg_layer_priv_key_shard.first);
	ownerRegLayerShards.emplace_back(std::move(ms->reg_layer_priv_key_shard));
	ownerOwnerLayerShardsIDs.push_back(ms->owner_layer_priv_key_shard.first);
	ownerOwnerLayerShards.emplace_back(std::move(ms->owner_layer_priv_key_shard));

	// each involved member should get its own shard(s) and register to use later
	for (auto& packetHandler : involvedRegMemberPacketHandlers)
	{
		auto up = post<pkt::UpdateResponse>(packetHandler, pkt::UpdateRequest{});
		EXPECT_TRUE(up.has_value());
		EXPECT_EQ(up->added_as_reg_member.size(), 1);
		EXPECT_EQ(up->added_as_reg_member.back().user_set_id, usersetID);
		EXPECT_EQ(up->added_as_reg_member.back().reg_layer_pub_key, pubRegLayerKey);
		EXPECT_EQ(up->added_as_reg_member.back().owner_layer_pub_key, pubOwnerLayerKey);
		regMemberShardsIDs.push_back(up->added_as_reg_member.back().reg_layer_priv_key_shard.first);
		regMemberShards.emplace_back(std::move(up->added_as_reg_member.back().reg_layer_priv_key_shard));
	}
	for (auto& packetHandler : nonCreatorInvolvedOwnerPacketHandlers)
	{
		auto up = post<pkt::UpdateResponse>(packetHandler, pkt::UpdateRequest{});
		EXPECT_TRUE(up.has_value());
		EXPECT_EQ(up->added_as_owner.size(), 1);
		EXPECT_EQ(up->added_as_owner.back().user_set_id, usersetID);
		EXPECT_EQ(up->added_as_owner.back().reg_layer_pub_key, pubRegLayerKey);
		EXPECT_EQ(up->added_as_owner.back().owner_layer_pub_key, pubOwnerLayerKey);
		ownerRegLayerShardsIDs.push_back(up->added_as_owner.back().reg_layer_priv_key_shard.first);
		ownerRegLayerShards.emplace_back(std::move(up->added_as_owner.back().reg_layer_priv_key_shard));
		ownerOwnerLayerShardsIDs.push_back(up->added_as_owner.back().owner_layer_priv_key_shard.first);
		ownerOwnerLayerShards.emplace_back(std::move(up->added_as_owner.back().owner_layer_priv_key_shard));
	}

	// as for the uninvolved users, they do the same, but we don't care about their shards
	for (auto& packetHandler : uninvolvedRegMemberPacketHandlers)
	{
		auto up = post<pkt::UpdateResponse>(packetHandler, pkt::UpdateRequest{});
		EXPECT_TRUE(up.has_value());
		EXPECT_EQ(up->added_as_reg_member.size(), 1);
		EXPECT_EQ(up->added_as_reg_member.back().user_set_id, usersetID);
		EXPECT_EQ(up->added_as_reg_member.back().reg_layer_pub_key, pubRegLayerKey);
		EXPECT_EQ(up->added_as_reg_member.back().owner_layer_pub_key, pubOwnerLayerKey);
	}
	for (auto& packetHandler : uninvolvedOwnerPacketHandlers)
	{
		auto up = post<pkt::UpdateResponse>(packetHandler, pkt::UpdateRequest{});
		EXPECT_TRUE(up.has_value());
		EXPECT_EQ(up->added_as_owner.size(), 1);
		EXPECT_EQ(up->added_as_owner.back().user_set_id, usersetID);
		EXPECT_EQ(up->added_as_owner.back().reg_layer_pub_key, pubRegLayerKey);
		EXPECT_EQ(up->added_as_owner.back().owner_layer_pub_key, pubOwnerLayerKey);
	}

	// encryption-decryption rounds loop
	auto involvedOwnerDist = Random<std::size_t>::get_dist_below(
		static_cast<std::size_t>(params.ownersThreshold) + 1
	); // see use below
	Schema schema;
	for (int i = 0; i < params.rounds; ++i)
	{
		// encrypt message
		const Buffer msg = senc::utils::random_bytes(params.msgSize);
		auto ciphertext = schema.encrypt(msg, pubRegLayerKey, pubOwnerLayerKey);

		// select random user to request decryption (for test)
		// (index 0 for set creator, after that for other owners - 
		//  we do this to match enumeration indexes of involvedOwnerSocks)
		const auto initiatorIndex = involvedOwnerDist();
		auto& initiatorPacketHandler = (0 == initiatorIndex) ? creatorPacketHandler
			: nonCreatorInvolvedOwnerPacketHandlers[initiatorIndex - 1];

		// initiator counts as a non-owner for the decryption of layer1
		regMemberShardsIDs.push_back(ownerRegLayerShardsIDs[initiatorIndex]);

		// 1) initiator starts decryption
		auto dc = post<pkt::DecryptResponse>(initiatorPacketHandler, pkt::DecryptRequest{
			usersetID, ciphertext
		});
		EXPECT_TRUE(dc.has_value());
		const OperationID opid = std::move(dc->op_id);

		// 2) all involved members run update to get decryption lookup request
		//    (uninvolved members are in lookup too, just won't be used later)
		for (auto [i, packetHandler] : memberPacketHandlers | enumerate)
		{
			if (initiatorIndex == i)
				continue; // initiator doesn't run update
			auto up = post<pkt::UpdateResponse>(packetHandler, pkt::UpdateRequest{});
			EXPECT_TRUE(up.has_value());
			EXPECT_EQ(up->on_lookup.size(), 1);
			EXPECT_EQ(up->on_lookup.back(), opid);
		}

		// 3) involved members tell server that they're willing to participate in operation
		for (auto [i, packetHandler] : involvedOwnerPacketHandlers | enumerate)
		{
			if (initiatorIndex == i)
				continue; // initiator doesn't request participance
			auto dp = post<pkt::DecryptParticipateResponse>(packetHandler, pkt::DecryptParticipateRequest{
				opid
			});
			EXPECT_TRUE(dp.has_value());
			EXPECT_EQ(dp->status, pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart);
		}
		for (auto& packetHandler : involvedRegMemberPacketHandlers)
		{
			auto dp = post<pkt::DecryptParticipateResponse>(packetHandler, pkt::DecryptParticipateRequest{
				opid
			});
			EXPECT_TRUE(dp.has_value());
			EXPECT_EQ(dp->status, pkt::DecryptParticipateResponse::Status::SendRegLayerPart);
		}

		// (and non-involved members are not required...)
		for (auto& packetHandler : uninvolvedPacketHandlers)
		{
			auto dp = post<pkt::DecryptParticipateResponse>(packetHandler, pkt::DecryptParticipateRequest{
				opid
			});
			EXPECT_TRUE(dp.has_value());
			EXPECT_EQ(dp->status, pkt::DecryptParticipateResponse::Status::NotRequired);
		}

		// 4) involved members run update to get decryption request
		for (auto [i, packetHandler] : involvedOwnerPacketHandlers | enumerate)
		{
			if (initiatorIndex == i)
				continue; // initiator doesn't run update
			auto up = post<pkt::UpdateResponse>(packetHandler, pkt::UpdateRequest{});
			EXPECT_TRUE(up.has_value());
			EXPECT_EQ(up->to_decrypt.size(), 1);
			EXPECT_EQ(up->to_decrypt.back().ciphertext, ciphertext);
			EXPECT_EQ(up->to_decrypt.back().op_id, opid);
			EXPECT_SAME_ELEMS(up->to_decrypt.back().shards_ids, ownerOwnerLayerShardsIDs);
		}
		for (auto& packetHandler : involvedRegMemberPacketHandlers)
		{
			auto up = post<pkt::UpdateResponse>(packetHandler, pkt::UpdateRequest{});
			EXPECT_TRUE(up.has_value());
			EXPECT_EQ(up->to_decrypt.size(), 1);
			EXPECT_EQ(up->to_decrypt.back().ciphertext, ciphertext);
			EXPECT_EQ(up->to_decrypt.back().op_id, opid);
			EXPECT_SAME_ELEMS(up->to_decrypt.back().shards_ids, regMemberShardsIDs);
		}

		// 5,6) involved memebrs compute decryption part locally and send them back
		std::vector<DecryptionPart> regLayerParts, ownerLayerParts;
		for (auto [i, phAndShard] : zip(involvedOwnerPacketHandlers, ownerOwnerLayerShards) | enumerate)
		{
			auto& [packetHandler, shard] = phAndShard;
			if (initiatorIndex == i)
				continue; // initiator doesn't compute yet

			auto part = senc::Shamir::decrypt_get_2l<OWNER_LAYER>( // owner knows it's layer2
				ciphertext,
				shard,
				ownerOwnerLayerShardsIDs
			);
			ownerLayerParts.push_back(part);

			auto sp = post<pkt::SendDecryptionPartResponse>(packetHandler, pkt::SendDecryptionPartRequest{
				.op_id = opid,
				.decryption_part = std::move(part)
			});
			EXPECT_TRUE(sp.has_value());
		}
		for (auto [packetHandler, shard] : zip(involvedRegMemberPacketHandlers, regMemberShards))
		{
			auto part = senc::Shamir::decrypt_get_2l<REG_LAYER>( // non-owner knows it's layer1
				ciphertext,
				shard,
				regMemberShardsIDs
			);
			regLayerParts.push_back(part);

			auto sp = post<pkt::SendDecryptionPartResponse>(packetHandler, pkt::SendDecryptionPartRequest{
				.op_id = opid,
				.decryption_part = std::move(part)
			});
			EXPECT_TRUE(sp.has_value());
		}

		// 7) initiator runs update to get finished decryption parts
		auto up = post<pkt::UpdateResponse>(initiatorPacketHandler, pkt::UpdateRequest{});
		EXPECT_TRUE(up.has_value());
		EXPECT_EQ(up->finished_decryptions.size(), 1);
		EXPECT_TRUE(up->finished_decryptions.back().op_id == opid);
		EXPECT_EQ(up->finished_decryptions.back().reg_layer_parts, regLayerParts);
		EXPECT_EQ(up->finished_decryptions.back().owner_layer_parts, ownerLayerParts);

		// check same shard IDs as involved members
		auto& finishedRegLayerShardsIDs = up->finished_decryptions.back().reg_layer_shards_ids;
		auto& finishedOwnerLayerShardsIDs = up->finished_decryptions.back().owner_layer_shards_ids;
		EXPECT_SAME_ELEMS(up->finished_decryptions.back().reg_layer_shards_ids, regMemberShardsIDs);
		EXPECT_SAME_ELEMS(up->finished_decryptions.back().owner_layer_shards_ids, ownerOwnerLayerShardsIDs);

		// 8) initiator computes their own decryption parts
		auto initiatorRegLayerPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
			ciphertext,
			ownerRegLayerShards[initiatorIndex],
			finishedRegLayerShardsIDs
		);
		auto initiatorOwnerLayerPart = senc::Shamir::decrypt_get_2l<OWNER_LAYER>(
			ciphertext,
			ownerOwnerLayerShards[initiatorIndex],
			finishedOwnerLayerShardsIDs
		);

		// 9) initiator combines their parts with received parts
		regLayerParts.push_back(std::move(initiatorRegLayerPart));
		ownerLayerParts.push_back(std::move(initiatorOwnerLayerPart));
		auto decrypted = senc::Shamir::decrypt_join_2l(
			ciphertext, regLayerParts, ownerLayerParts
		);
		EXPECT_EQ(decrypted, msg);

		// end of round
		regMemberShardsIDs.pop_back(); // remove initiator's shard ID
	}

	// logout
	for (auto& packetHandler : allPacketHandlers)
	{
		std::optional<pkt::LogoutResponse> lo =
			post<pkt::LogoutResponse>(packetHandler, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

// ===== Instantiation of Parameterized Tests =====

const auto SERVER_IMPLS = testing::Values(
	ServerTestParams{
		[](Port port) { return std::make_unique<senc::utils::TcpSocket<IPv4>>(IPv4::loopback(), port); },
		[](auto&&... args) { return new_server<IPv4>(args...); },
		std::make_unique<ShortTermServerStorage>,
		std::make_unique<PacketHandlerImplFactory<InlinePacketHandler>>
	},
	ServerTestParams{
		[](Port port) { return std::make_unique<senc::utils::TcpSocket<IPv4>>(IPv4::loopback(), port); },
		[](auto&&... args) { return new_server<IPv4>(args...); },
		std::make_unique<ShortTermServerStorage>,
		std::make_unique<PacketHandlerImplFactory<EncryptedPacketHandler>>
	},
	ServerTestParams{
		[](Port port) { return std::make_unique<senc::utils::TcpSocket<IPv6>>(IPv6::loopback(), port); },
		[](auto&&... args) { return new_server<IPv6>(args...); },
		std::make_unique<ShortTermServerStorage>,
		std::make_unique<PacketHandlerImplFactory<EncryptedPacketHandler>>
	}
);

const auto CYCLE_PARAMS = testing::Values(
	CycleParams{
		.owners              = 0,
		.regMembers          = 0,
		.nonMembers          = 0,
		.ownersThreshold     = 0,
		.regMembersThreshold = 0,
		.msgSize             = 256,
		.rounds              = 3
	},
	CycleParams{
		.owners              = 0,
		.regMembers          = 1,
		.nonMembers          = 0,
		.ownersThreshold     = 0,
		.regMembersThreshold = 1,
		.msgSize             = 256,
		.rounds              = 3
	},
	CycleParams{
		.owners              = 1,
		.regMembers          = 0,
		.nonMembers          = 0,
		.ownersThreshold     = 1,
		.regMembersThreshold = 0,
		.msgSize             = 256,
		.rounds              = 3
	},
	CycleParams{
		.owners              = 1,
		.regMembers          = 1,
		.nonMembers          = 0,
		.ownersThreshold     = 1,
		.regMembersThreshold = 1,
		.msgSize             = 256,
		.rounds              = 3
	},
	CycleParams{
		.owners              = 2,
		.regMembers          = 2,
		.nonMembers          = 1,
		.ownersThreshold     = 1,
		.regMembersThreshold = 1,
		.msgSize             = 256,
		.rounds              = 3
	},
	CycleParams{
		.owners              = 8,
		.regMembers          = 15,
		.nonMembers          = 7,
		.ownersThreshold     = 5,
		.regMembersThreshold = 10,
		.msgSize             = 256,
		.rounds              = 5
	}
);

INSTANTIATE_TEST_SUITE_P(
	ServerImplementations,
	ServerTest,
	SERVER_IMPLS
);

INSTANTIATE_TEST_SUITE_P(
	ServerMultiCycle,
	MultiCycleServerTest,
	testing::Combine(SERVER_IMPLS, CYCLE_PARAMS)
);

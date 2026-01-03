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
using senc::server::IServer;
using senc::server::Server;
using senc::PacketReceiver;
using senc::DecryptionPart;
using senc::PrivKeyShardID;
using senc::utils::Random;
using senc::utils::Buffer;
using senc::PrivKeyShard;
using senc::PacketSender;
using senc::OperationID;
using senc::utils::Port;
using senc::Schema;

using Socket = senc::utils::TcpSocket<senc::utils::IPv4>;

using senc::member_count_t;

using senc::OWNER_LAYER;
using senc::REG_LAYER;

using senc::utils::views::enumerate;
using senc::utils::views::join;
using senc::utils::views::zip;

// factory function types for creating member implementations
using StorageFactory = std::function<std::unique_ptr<IServerStorage>()>;
using ReceiverFactory = std::function<std::unique_ptr<PacketReceiver>()>;
using SenderFactory = std::function<std::unique_ptr<PacketSender>()>;
using ServerFactory = std::function<std::unique_ptr<IServer>(
	Port, Schema&, IServerStorage&, PacketReceiver&, PacketSender&,
	UpdateManager&, DecryptionsManager&
)>;

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
	ServerFactory serverFactory;
	StorageFactory storageFactory;
	ReceiverFactory receiverFactory;
	SenderFactory senderFactory;
};

class ServerTestBase : public testing::Test
{
protected:
	Port port;
	Schema schema;
	UpdateManager updateManager;
	DecryptionsManager decryptionsManager;
	std::unique_ptr<IServerStorage> storage;
	std::unique_ptr<PacketReceiver> receiver;
	std::unique_ptr<PacketSender> sender;
	std::unique_ptr<IServer> server;

	virtual const ServerTestParams& get_server_test_params() = 0;

	void SetUp() override
	{
		port = Random<Port>::sample_from_range(49152, 65535);
		const auto& params = get_server_test_params();
		storage = params.storageFactory();
		receiver = params.receiverFactory();
		sender = params.senderFactory();
		server = params.serverFactory(
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

	void make_connection(senc::utils::Socket& sock) const
	{
		sender->send_connection_request(sock);
		const bool validConn = receiver->recv_connection_response(sock);
		EXPECT_TRUE(validConn);
	}

	template <typename Response>
	auto post(senc::utils::Socket& sock, const auto& request) const
	{
		sender->send_request(sock, request);
		return receiver->recv_response<Response>(sock);
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
	auto client = Socket("127.0.0.1", port);
	make_connection(client);
	auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
	EXPECT_TRUE(lo.has_value());
}

TEST_P(ServerTest, SignupAndLogin)
{
	auto avi = Socket("127.0.0.1", port);
	auto batya = Socket("127.0.0.1", port);

	make_connection(avi);
	make_connection(batya);

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
	make_connection(avi);
	make_connection(batya);
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

	make_connection(client1);
	make_connection(client2);
	make_connection(client3);

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
		EXPECT_CONTAINS(gs->user_sets_ids, usersetID);

		// get members
		auto gm = post<pkt::GetMembersResponse>(client, pkt::GetMembersRequest{ usersetID });
		EXPECT_TRUE(gm.has_value());

		// check that u1 and u3 are owners
		for (const auto& owner : { u1, u3 })
			EXPECT_CONTAINS(gm->owners, owner);

		// check that u2 is a (regular) member
		EXPECT_CONTAINS(gm->reg_members, u2);
	}

	// logout
	for (auto& client : { std::ref(client1), std::ref(client2), std::ref(client3) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, MakeSetCheckKey)
{
	auto client1 = Socket("127.0.0.1", port);
	auto client2 = Socket("127.0.0.1", port);
	auto client3 = Socket("127.0.0.1", port);

	make_connection(client1);
	make_connection(client2);
	make_connection(client3);

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

	for (auto& client : { std::ref(client2), std::ref(client3) })
	{
		// get userset update, check same userset ID, get shard
		auto up = post<pkt::UpdateResponse>(client, pkt::UpdateRequest{});
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
	for (auto& client : { std::ref(client1), std::ref(client2), std::ref(client3) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, EmptyUpdateCycle)
{
	auto client = Socket("127.0.0.1", port);

	make_connection(client);

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

	make_connection(owner);
	make_connection(member);

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
	EXPECT_EQ(memberSetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(memberSetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& memberShard = memberSetsAddedTo.front().reg_layer_priv_key_shard;

	//    member has one operation to participate in, check same as owner's
	EXPECT_EQ(memberOnLookup.size(), 1);
	EXPECT_EQ(memberOnLookup.front(), ownerOpid);

	// 3) member tells server that they're willing to participate in operation
	auto dp = post<pkt::DecryptParticipateResponse>(member, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp.has_value() && dp->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

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
	auto memberPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
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
	for (auto& client : { std::ref(owner), std::ref(member) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, DecryptFlowTwoMembers)
{
	auto owner = Socket("127.0.0.1", port);
	auto member = Socket("127.0.0.1", port);
	auto member2 = Socket("127.0.0.1", port);

	make_connection(owner);
	make_connection(member);
	make_connection(member2);

	// signup
	auto su1 = post<pkt::SignupResponse>(owner, pkt::SignupRequest{ "owner" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(member, pkt::SignupRequest{ "member" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	auto su3 = post<pkt::SignupResponse>(member2, pkt::SignupRequest{ "member2" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=1
	auto ms = post<pkt::MakeUserSetResponse>(owner, pkt::MakeUserSetRequest{
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
	auto dc = post<pkt::DecryptResponse>(owner, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) member runs update to get decryption lookup request
	auto up1a = post<pkt::UpdateResponse>(member, pkt::UpdateRequest{});
	EXPECT_TRUE(up1a.has_value());
	const auto& memberSetsAddedTo = up1a->added_as_reg_member;
	const auto& memberOnLookup = up1a->on_lookup;

	auto up1b = post<pkt::UpdateResponse>(member2, pkt::UpdateRequest{});
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
	auto dp1 = post<pkt::DecryptParticipateResponse>(member, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp1.has_value() && dp1->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

	auto dp2 = post<pkt::DecryptParticipateResponse>(member2, pkt::DecryptParticipateRequest{
		member2OnLookup.front()
	});
	EXPECT_TRUE(dp2.has_value() && dp2->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

	// 4) members run update to get decryption request
	auto up2a = post<pkt::UpdateResponse>(member, pkt::UpdateRequest{});
	EXPECT_TRUE(up2a.has_value());
	const auto& memberToDecrypt = up2a->to_decrypt;

	auto up2b = post<pkt::UpdateResponse>(member2, pkt::UpdateRequest{});
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
	auto sp1 = post<pkt::SendDecryptionPartResponse>(member, pkt::SendDecryptionPartRequest{
		.op_id = memberOpid,
		.decryption_part = memberPart
	});
	EXPECT_TRUE(sp1.has_value());

	auto sp2 = post<pkt::SendDecryptionPartResponse>(member2, pkt::SendDecryptionPartRequest{
		.op_id = member2Opid,
		.decryption_part = member2Part
	});
	EXPECT_TRUE(sp2.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(owner, pkt::UpdateRequest{});
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
	for (auto& client : { std::ref(owner), std::ref(member), std::ref(member2) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, DecryptFlowExtraMember)
{
	auto owner = Socket("127.0.0.1", port);
	auto member = Socket("127.0.0.1", port);
	auto extra = Socket("127.0.0.1", port);

	make_connection(owner);
	make_connection(member);
	make_connection(extra);

	// signup
	auto su1 = post<pkt::SignupResponse>(owner, pkt::SignupRequest{ "owner" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(member, pkt::SignupRequest{ "member" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	auto su3 = post<pkt::SignupResponse>(extra, pkt::SignupRequest{ "extra" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=1
	auto ms = post<pkt::MakeUserSetResponse>(owner, pkt::MakeUserSetRequest{
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
	EXPECT_EQ(memberSetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(memberSetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& memberShard = memberSetsAddedTo.front().reg_layer_priv_key_shard;

	//    member has one operation to participate in, check same as owner's
	EXPECT_EQ(memberOnLookup.size(), 1);
	EXPECT_EQ(memberOnLookup.front(), ownerOpid);

	// (extra does same, but will not participate in decryption)
	auto upe = post<pkt::UpdateResponse>(extra, pkt::UpdateRequest{});
	EXPECT_TRUE(upe.has_value());
	const auto& extraSetsAddedTo = upe->added_as_reg_member;
	EXPECT_EQ(extraSetsAddedTo.size(), 1);
	EXPECT_EQ(extraSetsAddedTo.front().user_set_id, ownerUsersetID);
	EXPECT_EQ(extraSetsAddedTo.front().reg_layer_pub_key, ownerPubRegLayerKey);
	EXPECT_EQ(extraSetsAddedTo.front().owner_layer_pub_key, ownerPubOwnerLayerKey);
	const auto& extraShard = extraSetsAddedTo.front().reg_layer_priv_key_shard;
	(void)extraShard; // for debugging purposes

	// 3) member tells server that they're willing to participate in operation
	auto dp = post<pkt::DecryptParticipateResponse>(member, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp.has_value() && dp->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

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
	auto memberPart = senc::Shamir::decrypt_get_2l<REG_LAYER>(
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
	for (auto& client : { std::ref(owner), std::ref(member), std::ref(extra) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, DecryptFlow2L)
{
	auto owner = Socket("127.0.0.1", port);
	auto member = Socket("127.0.0.1", port);
	auto owner2 = Socket("127.0.0.1", port);

	make_connection(owner);
	make_connection(member);
	make_connection(owner2);

	// signup
	auto su1 = post<pkt::SignupResponse>(owner, pkt::SignupRequest{ "owner" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(member, pkt::SignupRequest{ "member" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	auto su3 = post<pkt::SignupResponse>(owner2, pkt::SignupRequest{ "owner2" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=1
	auto ms = post<pkt::MakeUserSetResponse>(owner, pkt::MakeUserSetRequest{
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
	auto dc = post<pkt::DecryptResponse>(owner, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) members run update to get decryption lookup request
	auto up1 = post<pkt::UpdateResponse>(member, pkt::UpdateRequest{});
	EXPECT_TRUE(up1.has_value());
	const auto& memberSetsAddedTo = up1->added_as_reg_member;
	const auto& memberOnLookup = up1->on_lookup;

	auto up1b = post<pkt::UpdateResponse>(owner2, pkt::UpdateRequest{});
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
	auto dp = post<pkt::DecryptParticipateResponse>(member, pkt::DecryptParticipateRequest{
		memberOnLookup.front()
	});
	EXPECT_TRUE(dp.has_value() && dp->status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart);

	auto dp2 = post<pkt::DecryptParticipateResponse>(owner2, pkt::DecryptParticipateRequest{
		owner2OnLookup.front()
	});
	EXPECT_TRUE(dp2.has_value() && dp2->status == pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart);

	// 4) members run update to get decryption request
	auto up2 = post<pkt::UpdateResponse>(member, pkt::UpdateRequest{});
	EXPECT_TRUE(up2.has_value());
	const auto& memberToDecrypt = up2->to_decrypt;

	auto up2b = post<pkt::UpdateResponse>(owner2, pkt::UpdateRequest{});
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
	auto sp = post<pkt::SendDecryptionPartResponse>(member, pkt::SendDecryptionPartRequest{
		.op_id = memberOpid,
		.decryption_part = memberPart
	});
	EXPECT_TRUE(sp.has_value());

	auto sp2 = post<pkt::SendDecryptionPartResponse>(owner2, pkt::SendDecryptionPartRequest{
		.op_id = owner2Opid,
		.decryption_part = owner2Part
	});
	EXPECT_TRUE(sp2.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(owner, pkt::UpdateRequest{});
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
	for (auto& client : { std::ref(owner), std::ref(member), std::ref(owner2) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(ServerTest, DecryptFlowOwnersOnly)
{
	auto owner = Socket("127.0.0.1", port);
	auto owner2 = Socket("127.0.0.1", port);
	auto owner3 = Socket("127.0.0.1", port);

	make_connection(owner);
	make_connection(owner2);
	make_connection(owner3);

	// signup
	auto su1 = post<pkt::SignupResponse>(owner, pkt::SignupRequest{ "owner" });
	EXPECT_TRUE(su1.has_value() && su1->status == pkt::SignupResponse::Status::Success);
	auto su2 = post<pkt::SignupResponse>(owner2, pkt::SignupRequest{ "owner2" });
	EXPECT_TRUE(su2.has_value() && su2->status == pkt::SignupResponse::Status::Success);
	auto su3 = post<pkt::SignupResponse>(owner3, pkt::SignupRequest{ "owner3" });
	EXPECT_TRUE(su3.has_value() && su3->status == pkt::SignupResponse::Status::Success);

	// make set with threshold=2
	auto ms = post<pkt::MakeUserSetResponse>(owner, pkt::MakeUserSetRequest{
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
	auto dc = post<pkt::DecryptResponse>(owner, pkt::DecryptRequest{
		ownerUsersetID,
		ownerCiphertext
	});
	EXPECT_TRUE(dc.has_value());
	const auto& ownerOpid = dc->op_id;

	// 2) members run update to get decryption lookup request
	auto up1a = post<pkt::UpdateResponse>(owner2, pkt::UpdateRequest{});
	EXPECT_TRUE(up1a.has_value());
	const auto& owner2SetsAddedTo = up1a->added_as_owner;
	const auto& owner2OnLookup = up1a->on_lookup;

	auto up1b = post<pkt::UpdateResponse>(owner3, pkt::UpdateRequest{});
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
	auto dp1 = post<pkt::DecryptParticipateResponse>(owner2, pkt::DecryptParticipateRequest{
		owner2OnLookup.front()
	});
	EXPECT_TRUE(dp1.has_value() && dp1->status == pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart);

	auto dp2 = post<pkt::DecryptParticipateResponse>(owner3, pkt::DecryptParticipateRequest{
		owner3OnLookup.front()
	});
	EXPECT_TRUE(dp2.has_value() && dp2->status == pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart);

	// 4) members run update to get decryption request
	auto up2a = post<pkt::UpdateResponse>(owner2, pkt::UpdateRequest{});
	EXPECT_TRUE(up2a.has_value());
	const auto& owner2ToDecrypt = up2a->to_decrypt;

	auto up2b = post<pkt::UpdateResponse>(owner3, pkt::UpdateRequest{});
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
	auto sp1 = post<pkt::SendDecryptionPartResponse>(owner2, pkt::SendDecryptionPartRequest{
		.op_id = owner2Opid,
		.decryption_part = owner2Part
	});
	EXPECT_TRUE(sp1.has_value());

	auto sp2 = post<pkt::SendDecryptionPartResponse>(owner3, pkt::SendDecryptionPartRequest{
		.op_id = owner3Opid,
		.decryption_part = owner3Part
	});
	EXPECT_TRUE(sp2.has_value());

	// 7) owner runs update to get finished decryption parts
	auto up3 = post<pkt::UpdateResponse>(owner, pkt::UpdateRequest{});
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
	for (auto& client : { std::ref(owner), std::ref(owner2), std::ref(owner3) })
	{
		auto lo = post<pkt::LogoutResponse>(client, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

TEST_P(MultiCycleServerTest, MultiCycleDecryptFlow2L)
{
	auto makeusers = [this](std::size_t size, const char* prefix)
	{
		std::vector<Socket> sockets;
		std::vector<std::string> usernames;
		for (std::size_t i = 0; i < size; ++i)
		{
			sockets.emplace_back("127.0.0.1", port);
			usernames.push_back(prefix + std::to_string(i));
		}
		return std::make_tuple(
			std::move(sockets),
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

	std::vector<Socket> creatorSocksVec;
	creatorSocksVec.emplace_back("127.0.0.1", port);
	std::vector<std::string> creatorUsernamesVec = { "creator" };

	auto creatorSocks = creatorSocksVec | std::views::all;
	auto creatorUsernames = creatorUsernamesVec | std::views::all;

	auto& creatorSock = creatorSocksVec.front();

	auto [nonCreatorOwnerSocksVec, nonCreatorOwnerUsernamesVec] = makeusers(params.owners, "owner");
	auto nonCreatorOwnerSocks = nonCreatorOwnerSocksVec | std::views::all;
	auto nonCreatorOwnerUsernames = nonCreatorOwnerUsernamesVec | std::views::all;
	auto [regMemberSocksVec, regMemberUsernamesVec] = makeusers(params.regMembers, "reg");
	auto regMemberSocks = regMemberSocksVec | std::views::all;
	auto regMemberUsernames = regMemberUsernamesVec | std::views::all;
	auto [nonMemberSocksVec, nonMemberUsernamesVec] = makeusers(params.nonMembers, "foreign");
	auto nonMemberSocks = nonMemberSocksVec | std::views::all;
	auto nonMemberUsernames = nonMemberUsernamesVec | std::views::all;

	auto nonCreatorInvolvedOwnerSocks = nonCreatorOwnerSocks |
		std::views::take(params.ownersThreshold);
	// auto nonCreatorInvolvedOwnerUsernames = nonCreatorOwnerUsernames |
	// 	std::views::take(params.regMembersThreshold);

	auto involvedOwnerSocks = join(creatorSocks, nonCreatorInvolvedOwnerSocks);
	// auto involvedOwnerUsernames = join(creatorUsernames, nonCreatorInvolvedOwnerUsernames);

	auto uninvolvedOwnerSocks = nonCreatorOwnerSocks | std::views::drop(params.ownersThreshold);
	// auto uninvolvedOwnerUsernames = nonCreatorOwnerUsernames | std::views::drop(params.regMembersThreshold);

	auto involvedRegMemberSocks = regMemberSocks | std::views::take(params.regMembersThreshold);
	// auto involvedRegMemberUsernames = regMemberUsernames | std::views::take(params.regMembersThreshold);

	auto uninvolvedRegMemberSocks = regMemberSocks | std::views::drop(params.regMembersThreshold);
	// auto uninvolvedRegMemberUsernames = regMemberUsernames | std::views::drop(params.regMembersThreshold);

	// auto involvedSocks = join(involvedOwnerSocks, involvedRegMemberSocks);
	auto uninvolvedSocks = join(uninvolvedOwnerSocks, uninvolvedRegMemberSocks);

	auto memberSocks = join(creatorSocks, nonCreatorOwnerSocks, regMemberSocks);
	auto memberUsernames = join(creatorUsernames, nonCreatorOwnerUsernames, regMemberUsernames);

	auto allSocks = join(memberSocks, nonMemberSocks);
	auto allUsernames = join(memberUsernames, nonMemberUsernames);

	for (auto& sock : allSocks)
		make_connection(sock);

	// signup
	for (auto [sock, username] : zip(allSocks, allUsernames))
	{
		std::optional<pkt::SignupResponse> su = 
			post<pkt::SignupResponse>(sock, pkt::SignupRequest{ username });
		EXPECT_TRUE(su.has_value());
		EXPECT_EQ(su->status, pkt::SignupResponse::Status::Success);
	}

	// vectors to store shards later
	std::vector<PrivKeyShard> regMemberShards;
	std::vector<PrivKeyShardID> regMemberShardsIDs;
	std::vector<PrivKeyShard> ownerRegLayerShards, ownerOwnerLayerShards;
	std::vector<PrivKeyShardID> ownerRegLayerShardsIDs, ownerOwnerLayerShardsIDs;

	// make userset
	auto ms = post<pkt::MakeUserSetResponse>(creatorSock, pkt::MakeUserSetRequest{
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
	for (auto& sock : involvedRegMemberSocks)
	{
		auto up = post<pkt::UpdateResponse>(sock, pkt::UpdateRequest{});
		EXPECT_TRUE(up.has_value());
		EXPECT_EQ(up->added_as_reg_member.size(), 1);
		EXPECT_EQ(up->added_as_reg_member.back().user_set_id, usersetID);
		EXPECT_EQ(up->added_as_reg_member.back().reg_layer_pub_key, pubRegLayerKey);
		EXPECT_EQ(up->added_as_reg_member.back().owner_layer_pub_key, pubOwnerLayerKey);
		regMemberShardsIDs.push_back(up->added_as_reg_member.back().reg_layer_priv_key_shard.first);
		regMemberShards.emplace_back(std::move(up->added_as_reg_member.back().reg_layer_priv_key_shard));
	}
	for (auto& sock : nonCreatorInvolvedOwnerSocks)
	{
		auto up = post<pkt::UpdateResponse>(sock, pkt::UpdateRequest{});
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
	for (auto& sock : uninvolvedRegMemberSocks)
	{
		auto up = post<pkt::UpdateResponse>(sock, pkt::UpdateRequest{});
		EXPECT_TRUE(up.has_value());
		EXPECT_EQ(up->added_as_reg_member.size(), 1);
		EXPECT_EQ(up->added_as_reg_member.back().user_set_id, usersetID);
		EXPECT_EQ(up->added_as_reg_member.back().reg_layer_pub_key, pubRegLayerKey);
		EXPECT_EQ(up->added_as_reg_member.back().owner_layer_pub_key, pubOwnerLayerKey);
	}
	for (auto& sock : uninvolvedOwnerSocks)
	{
		auto up = post<pkt::UpdateResponse>(sock, pkt::UpdateRequest{});
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
		auto& initiator = (0 == initiatorIndex) ? creatorSock 
			: nonCreatorInvolvedOwnerSocks[initiatorIndex - 1];

		// initiator counts as a non-owner for the decryption of layer1
		regMemberShardsIDs.push_back(ownerRegLayerShardsIDs[initiatorIndex]);

		// 1) initiator starts decryption
		auto dc = post<pkt::DecryptResponse>(initiator, pkt::DecryptRequest{
			usersetID, ciphertext
		});
		EXPECT_TRUE(dc.has_value());
		const OperationID opid = std::move(dc->op_id);

		// 2) all involved members run update to get decryption lookup request
		//    (uninvolved members are in lookup too, just won't be used later)
		for (auto [i, sock] : memberSocks | enumerate)
		{
			if (initiatorIndex == i)
				continue; // initiator doesn't run update
			auto up = post<pkt::UpdateResponse>(sock, pkt::UpdateRequest{});
			EXPECT_TRUE(up.has_value());
			EXPECT_EQ(up->on_lookup.size(), 1);
			EXPECT_EQ(up->on_lookup.back(), opid);
		}

		// 3) involved members tell server that they're willing to participate in operation
		for (auto [i, sock] : involvedOwnerSocks | enumerate)
		{
			if (initiatorIndex == i)
				continue; // initiator doesn't request participance
			auto dp = post<pkt::DecryptParticipateResponse>(sock, pkt::DecryptParticipateRequest{
				opid
			});
			EXPECT_TRUE(dp.has_value());
			EXPECT_EQ(dp->status, pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart);
		}
		for (auto& sock : involvedRegMemberSocks)
		{
			auto dp = post<pkt::DecryptParticipateResponse>(sock, pkt::DecryptParticipateRequest{
				opid
			});
			EXPECT_TRUE(dp.has_value());
			EXPECT_EQ(dp->status, pkt::DecryptParticipateResponse::Status::SendRegLayerPart);
		}

		// (and non-involved members are not required...)
		for (auto& sock : uninvolvedSocks)
		{
			auto dp = post<pkt::DecryptParticipateResponse>(sock, pkt::DecryptParticipateRequest{
				opid
			});
			EXPECT_TRUE(dp.has_value());
			EXPECT_EQ(dp->status, pkt::DecryptParticipateResponse::Status::NotRequired);
		}

		// 4) involved members run update to get decryption request
		for (auto [i, sock] : involvedOwnerSocks | enumerate)
		{
			if (initiatorIndex == i)
				continue; // initiator doesn't run update
			auto up = post<pkt::UpdateResponse>(sock, pkt::UpdateRequest{});
			EXPECT_TRUE(up.has_value());
			EXPECT_EQ(up->to_decrypt.size(), 1);
			EXPECT_EQ(up->to_decrypt.back().ciphertext, ciphertext);
			EXPECT_EQ(up->to_decrypt.back().op_id, opid);
			EXPECT_SAME_ELEMS(up->to_decrypt.back().shards_ids, ownerOwnerLayerShardsIDs);
		}
		for (auto& sock : involvedRegMemberSocks)
		{
			auto up = post<pkt::UpdateResponse>(sock, pkt::UpdateRequest{});
			EXPECT_TRUE(up.has_value());
			EXPECT_EQ(up->to_decrypt.size(), 1);
			EXPECT_EQ(up->to_decrypt.back().ciphertext, ciphertext);
			EXPECT_EQ(up->to_decrypt.back().op_id, opid);
			EXPECT_SAME_ELEMS(up->to_decrypt.back().shards_ids, regMemberShardsIDs);
		}

		// 5,6) involved memebrs compute decryption part locally and send them back
		std::vector<DecryptionPart> regLayerParts, ownerLayerParts;
		for (auto [i, sockshard] : zip(involvedOwnerSocks, ownerOwnerLayerShards) | enumerate)
		{
			auto& [sock, shard] = sockshard;
			if (initiatorIndex == i)
				continue; // initiator doesn't compute yet

			auto part = senc::Shamir::decrypt_get_2l<OWNER_LAYER>( // owner knows it's layer2
				ciphertext,
				shard,
				ownerOwnerLayerShardsIDs
			);
			ownerLayerParts.push_back(part);

			auto sp = post<pkt::SendDecryptionPartResponse>(sock, pkt::SendDecryptionPartRequest{
				.op_id = opid,
				.decryption_part = std::move(part)
			});
			EXPECT_TRUE(sp.has_value());
		}
		for (auto [sock, shard] : zip(involvedRegMemberSocks, regMemberShards))
		{
			auto part = senc::Shamir::decrypt_get_2l<REG_LAYER>( // non-owner knows it's layer1
				ciphertext,
				shard,
				regMemberShardsIDs
			);
			regLayerParts.push_back(part);

			auto sp = post<pkt::SendDecryptionPartResponse>(sock, pkt::SendDecryptionPartRequest{
				.op_id = opid,
				.decryption_part = std::move(part)
			});
			EXPECT_TRUE(sp.has_value());
		}

		// 7) initiator runs update to get finished decryption parts
		auto up = post<pkt::UpdateResponse>(initiator, pkt::UpdateRequest{});
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
	for (auto& sock : allSocks)
	{
		std::optional<pkt::LogoutResponse> lo =
			post<pkt::LogoutResponse>(sock, pkt::LogoutRequest{});
		EXPECT_TRUE(lo.has_value());
	}
}

// ===== Instantiation of Parameterized Tests =====

const auto SERVER_IMPLS = testing::Values(
	ServerTestParams{
		[](auto&&... args) { return std::make_unique<Server>(args...); },
		std::make_unique<ShortTermServerStorage>,
		std::make_unique<InlinePacketReceiver>,
		std::make_unique<InlinePacketSender>
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

INSTANTIATE_TEST_CASE_P(
	ServerMultiCycle,
	MultiCycleServerTest,
	testing::Combine(SERVER_IMPLS, CYCLE_PARAMS)
);

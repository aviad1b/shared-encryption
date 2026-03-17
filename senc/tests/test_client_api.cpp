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
using senc::utils::HashMap;
using senc::utils::HashSet;
using senc::utils::Buffer;
using senc::utils::Port;
using senc::utils::IPv4;
using senc::OperationID;
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
	std::string profileBaseDir;
	const char* ip;

	void SetUp() override
	{
		profileBaseDir = temp_dir_path();
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

struct DecsMap
{
	HashMap<OperationID, std::vector<Buffer>> map;
	std::mutex mtx;

	DecsMap() = default;

	bool empty()
	{
		const std::lock_guard<std::mutex> lock(mtx);
		return map.empty();
	}
};

static void append_decs(const char* opid, const uint8_t* bytes, uint64_t len, uintptr_t context)
{
	auto* pDecsMap = reinterpret_cast<DecsMap*>(context);
	const std::lock_guard<std::mutex> lock(pDecsMap->mtx);
	pDecsMap->map[opid].emplace_back(bytes, bytes + len);
}

static void add_str_to_hash_set(const char* str, uintptr_t context)
{
	auto* hashSet = reinterpret_cast<HashSet<std::string>*>(context);
	hashSet->insert(str);
}

static void test_userset_id_eq(const char* id, const char* name, uintptr_t context)
{
	(void)name;
	const char* otherID = reinterpret_cast<const char*>(context);
	ASSERT_EQ(std::strcmp(id, otherID), 0);
}

struct test_userset_members_param_t
{
	const std::vector<const char*>& owners;
	const std::vector<const char*>& regs;
	std::size_t ownersFound, regsFound;

	test_userset_members_param_t(const std::vector<const char*>& owners,
								 const std::vector<const char*>& regs)
		: owners(owners), regs(regs), ownersFound(0), regsFound(0) {  }

	void test() const
	{
		ASSERT_EQ(ownersFound, owners.size());
		ASSERT_EQ(regsFound, regs.size());
	}
};

static void test_ctx_owners(const char* username, uintptr_t context)
{
	auto* param = reinterpret_cast<test_userset_members_param_t*>(context);

	// check exists
	ASSERT_NE(
		param->owners.end(),
		std::find(param->owners.begin(), param->owners.end(), std::string(username))
	);
	// (same count is checked in param test() method)

	param->ownersFound++;
}

static void test_ctx_regs(const char* username, uintptr_t context)
{
	auto* param = reinterpret_cast<test_userset_members_param_t*>(context);

	// check exists
	ASSERT_NE(
		param->regs.end(),
		std::find(param->regs.begin(), param->regs.end(), std::string(username))
	);
	// (same count is checked in param test() method)

	param->regsFound++;
}

void ASSERT_NO_ERROR(const SENC_Handle& handle)
{
	ASSERT_FALSE(SENC_HasError(handle));
}

TEST_F(ClientApiTest, SignupLoginLogout)
{
	SENC_Handle hClient = SENC_Connect(ip, port, nullptr, 0);

	const char* username = "user";
	const char* password = "pass123";

	ASSERT_NO_ERROR(SENC_SignUp(hClient, profileBaseDir.c_str(), username, password));

	ASSERT_NO_ERROR(SENC_LogOut(hClient));

	ASSERT_NO_ERROR(SENC_LogIn(hClient, profileBaseDir.c_str(), username, password));

	ASSERT_NO_ERROR(SENC_LogOut(hClient));

	SENC_Disconnect(hClient);
}

TEST_F(ClientApiTest, UserSearch)
{
	std::vector<std::pair<std::string, std::string>> users = {
		{ "aviad", "pass123" },
		{ "batya", "sadfg" },
		{ "avihay", "hdgsfa" },
		{ "gal", "2134" },
		{ "aviel123", "ASKDFHU6*$" },
		{ "dan", "ads" },
		{ "miavi", "gfsa" },
		{ "miavi2serethahemshech", "hdgs" }
	};

	// connect & signup each user
	std::vector<SENC_Handle> hClients;
	for (const auto& [username, password] : users)
	{
		hClients.emplace_back(SENC_Connect(ip, port, nullptr, 0));
		ASSERT_NO_ERROR(hClients.back());
		ASSERT_NO_ERROR(SENC_SignUp(
			hClients.back(), profileBaseDir.c_str(),
			username.c_str(), password.c_str()
		));
	}

	// try search as each user
	for (auto& hClient : hClients)
	{
		HashSet<std::string> usersFound;
		SENC_UserSearch(
			hClient, "avi", add_str_to_hash_set,
			reinterpret_cast<uintptr_t>(&usersFound)
		);
		EXPECT_EQ(usersFound.size(), 5);
		EXPECT_TRUE(usersFound.contains("aviad"));
		EXPECT_TRUE(usersFound.contains("avihay"));
		EXPECT_TRUE(usersFound.contains("aviel123"));
		EXPECT_TRUE(usersFound.contains("miavi"));
		EXPECT_TRUE(usersFound.contains("miavi2serethahemshech"));
	}

	// logout and disconnect all users
	for (auto& hClient : hClients)
	{
		ASSERT_NO_ERROR(SENC_LogOut(hClient));
		SENC_Disconnect(hClient);
	}
}

TEST_F(ClientApiTest, RoundTripFlow)
{
	DecsMap decs;

	// connect 4 clients
	SENC_Handle hClient1 = SENC_Connect(
		ip, port,
		append_decs,
		reinterpret_cast<uintptr_t>(&decs)
	);
	SENC_Handle hClient2 = SENC_Connect(ip, port, nullptr, 0);
	SENC_Handle hClient3 = SENC_Connect(ip, port, nullptr, 0);
	SENC_Handle hClient4 = SENC_Connect(ip, port, nullptr, 0);

	// signup 4 users
	ASSERT_NO_ERROR(SENC_SignUp(hClient1, profileBaseDir.c_str(), "aaa", "AAA"));
	ASSERT_NO_ERROR(SENC_SignUp(hClient2, profileBaseDir.c_str(), "bbb", "BBB"));
	ASSERT_NO_ERROR(SENC_SignUp(hClient3, profileBaseDir.c_str(), "ccc", "CCC"));
	ASSERT_NO_ERROR(SENC_SignUp(hClient4, profileBaseDir.c_str(), "ddd", "DDD"));

	// create userset where aaa,bbb are owners and ccc,ddd are non-owners
	std::vector<const char*> owners{ "bbb" };
	std::vector<const char*> allOwners{ "aaa", "bbb" }; // including group creator
	std::vector<const char*> regs{ "ccc", "ddd" };
	SENC_Handle hUserSetID = SENC_MakeUserSet(
		hClient1,
		owners.size(), regs.size(),
		owners.data(), regs.data(),
		1, 1,
		"some_name"
	);
	ASSERT_NO_ERROR(hUserSetID);
	const char* usersetID = SENC_GetStringValue(hUserSetID);

	// try getting usersets and check only have this new userset
	ASSERT_NO_ERROR(SENC_GetUserSets(hClient1, test_userset_id_eq, reinterpret_cast<uintptr_t>(usersetID)));
	ASSERT_NO_ERROR(SENC_GetUserSets(hClient2, test_userset_id_eq, reinterpret_cast<uintptr_t>(usersetID)));

	// try getting userset members and check equals
	test_userset_members_param_t testMembersParam(allOwners, regs);
	ASSERT_NO_ERROR(SENC_GetUserSetMembers(
		hClient1, usersetID, test_ctx_owners, test_ctx_regs,
		reinterpret_cast<uintptr_t>(&testMembersParam)
	));
	testMembersParam.test();

	// encrypt message
	const Buffer msg = senc::utils::random_bytes(256);
	SENC_Handle hCiphertext = SENC_Encrypt(
		hClient1, usersetID,
		reinterpret_cast<const uint8_t*>(msg.data()),
		msg.size()
	);
	ASSERT_NO_ERROR(hCiphertext);

	// queue message decrypt
	SENC_Handle hOPID = SENC_Decrypt(hClient1, usersetID, hCiphertext);
	ASSERT_NO_ERROR(hOPID);
	OperationID opid = SENC_GetStringValue(hOPID);

	// wait until decryption was added to decs
	while (decs.empty())
		std::this_thread::sleep_for(std::chrono::seconds(1));

	// check got decryption which is same as `msg`
	{
		const std::lock_guard<std::mutex> lock(decs.mtx);
		EXPECT_EQ(decs.map.size(), 1);
		auto& decsVec = decs.map.at(opid);
		EXPECT_EQ(decsVec.size(), 1);
		auto& result = decsVec.front();

		EXPECT_EQ(result, msg);
	}

	// logout all users
	ASSERT_NO_ERROR(SENC_LogOut(hClient1));
	ASSERT_NO_ERROR(SENC_LogOut(hClient2));
	ASSERT_NO_ERROR(SENC_LogOut(hClient3));
	ASSERT_NO_ERROR(SENC_LogOut(hClient4));

	// disconnect happens in destructor
}

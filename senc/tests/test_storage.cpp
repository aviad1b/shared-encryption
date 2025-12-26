#include <WinSock2.h> // no idea why this was needed here, microsoft are getting on my nerves
#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include "../server/IServerStorage.hpp"
#include "../server/ShortTermServerStorage.hpp"
#include "tests_utils.hpp"

using senc::server::ShortTermServerStorage;
using senc::server::IServerStorage;
using senc::server::OperationInfo;
using senc::server::UserSetInfo;
using senc::PrivKeyShardID;
using senc::member_count_t;
using senc::OperationID;
using senc::UserSetID;

using senc::utils::ranges::strings;

// factory function type for creating storage implementations
using StorageFactory = std::function<std::unique_ptr<IServerStorage>()>;

class ServerStorageTest : public ::testing::TestWithParam<StorageFactory>
{
protected:
	std::unique_ptr<IServerStorage> storage;

	void SetUp() override
	{
		// use factory from parameter to instantiate implementation
		storage = GetParam()();
	}

	void TearDown() override
	{
		storage.reset();
	}
};

// ----- User Management Tests -----

TEST_P(ServerStorageTest, UserExists_ReturnsFalseForNonExistentUser)
{
	EXPECT_FALSE(storage->user_exists("nonexistent_user"));
}

TEST_P(ServerStorageTest, UserExists_ReturnsTrueAfterUserCreation)
{
	const std::string username = "avi";

	storage->new_user(username);

	EXPECT_TRUE(storage->user_exists(username));
}

TEST_P(ServerStorageTest, NewUser_MultipleUsersCanBeCreated)
{
	storage->new_user("avi");
	storage->new_user("batya");
	storage->new_user("gal");

	EXPECT_TRUE(storage->user_exists("avi"));
	EXPECT_TRUE(storage->user_exists("batya"));
	EXPECT_TRUE(storage->user_exists("gal"));
}

// ----- UserSet Management Tests -----

TEST_P(ServerStorageTest, NewUserset_ReturnsValidUserSetID)
{
	storage->new_user("avi");
	storage->new_user("batya");

	auto owners = { "avi", "batya" };
	std::initializer_list<std::string> regMembers = {};

	storage->new_userset(strings(owners), strings(regMembers), 2, 0);
}

TEST_P(ServerStorageTest, NewUserset_WithRegularMembers)
{
	storage->new_user("avi");
	storage->new_user("batya");
	storage->new_user("gal");
	storage->new_user("dani");

	auto owners = { "avi", "batya" };
	auto regMembers = { "gal", "dani" };

	storage->new_userset(strings(owners), strings(regMembers), 2, 1);
}

TEST_P(ServerStorageTest, NewUserset_MultipleSetsReturnDifferentIDs)
{
	storage->new_user("avi");
	storage->new_user("batya");

	auto owners1 = { "avi" };
	auto owners2 = { "batya" };
	std::initializer_list<std::string> regMembers = {};

	UserSetID id1 = storage->new_userset(strings(owners1), strings(regMembers), 1, 0);
	UserSetID id2 = storage->new_userset(strings(owners2), strings(regMembers), 1, 0);

	EXPECT_NE(id1, id2);
}

TEST_P(ServerStorageTest, GetUsersetInfo_ReturnsCorrectConfiguration)
{
	storage->new_user("avi");
	storage->new_user("batya");
	storage->new_user("gal");

	auto owners = { "avi", "batya" };
	auto regMembers = { "gal" };
	member_count_t ownersThreshold = 2;
	member_count_t regThreshold = 1;

	UserSetID usersetID = storage->new_userset(
		strings(owners), strings(regMembers),
		ownersThreshold, regThreshold
	);

	UserSetInfo info = storage->get_userset_info(usersetID);

	EXPECT_EQ(info.owners.size(), 2);
	EXPECT_CONTAINS(info.owners, "avi");
	EXPECT_CONTAINS(info.owners, "batya");
	EXPECT_EQ(info.reg_members.size(), 1);
	EXPECT_CONTAINS(info.reg_members, "gal");
	EXPECT_EQ(info.owners_threshold, ownersThreshold);
	EXPECT_EQ(info.reg_members_threshold, regThreshold);
}

TEST_P(ServerStorageTest, GetUsersets_ReturnsEmptyForNewUser)
{
	storage->new_user("avi");

	auto usersets = storage->get_usersets("avi");

	EXPECT_TRUE(usersets.empty());
}

TEST_P(ServerStorageTest, GetUsersets_ReturnsUserSetsForOwner)
{
	storage->new_user("avi");
	storage->new_user("batya");

	auto owners1 = { "avi" };
	auto owners2 = { "avi", "batya" };
	std::initializer_list<std::string> regMembers = {};

	UserSetID id1 = storage->new_userset(strings(owners1), strings(regMembers), 1, 0);
	UserSetID id2 = storage->new_userset(strings(owners2), strings(regMembers), 2, 0);

	auto aviSets = storage->get_usersets("avi");

	EXPECT_EQ(aviSets.size(), 2);
	EXPECT_TRUE(aviSets.contains(id1));
	EXPECT_TRUE(aviSets.contains(id2));
}

TEST_P(ServerStorageTest, GetUsersets_DoesNotReturnSetsWhereUserIsOnlyRegularMember)
{
	storage->new_user("avi");
	storage->new_user("batya");

	auto owners = { "avi" };
	auto regMembers = { "batya" };

	storage->new_userset(strings(owners), strings(regMembers), 1, 1);

	auto batyaSets = storage->get_usersets("batya");

	EXPECT_TRUE(batyaSets.empty());
}

TEST_P(ServerStorageTest, UserOwnsUserset_ReturnsTrueForOwner)
{
	storage->new_user("avi");
	storage->new_user("batya");

	auto owners = { "avi", "batya" };
	std::initializer_list<std::string> regMembers = {};

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 2, 0);

	EXPECT_TRUE(storage->user_owns_userset("avi", usersetID));
	EXPECT_TRUE(storage->user_owns_userset("batya", usersetID));
}

TEST_P(ServerStorageTest, UserOwnsUserset_ReturnsFalseForNonOwner)
{
	storage->new_user("avi");
	storage->new_user("batya");

	auto owners = { "avi" };
	auto regMembers = { "batya" };

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 1, 1);

	EXPECT_TRUE(storage->user_owns_userset("avi", usersetID));
	EXPECT_FALSE(storage->user_owns_userset("batya", usersetID));
}

TEST_P(ServerStorageTest, UserOwnsUserset_ReturnsFalseForNonExistentUserset)
{
	storage->new_user("avi");

	UserSetID fakeID = UserSetID::generate(); // Assuming this doesn't exist

	EXPECT_FALSE(storage->user_owns_userset("avi", fakeID));
}

// ----- Shard ID Tests -----

TEST_P(ServerStorageTest, GetShardId_ReturnsValidShardID)
{
	storage->new_user("avi");

	auto owners = { "avi" };
	std::initializer_list<std::string> regMembers = {};

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 1, 0);

	PrivKeyShardID shardID = storage->get_shard_id("avi", usersetID);

	EXPECT_NE(shardID, PrivKeyShardID{});
}

TEST_P(ServerStorageTest, GetShardId_DifferentUsersGetDifferentShards)
{
	storage->new_user("avi");
	storage->new_user("batya");

	auto owners = { "avi", "batya" };
	std::initializer_list<std::string> regMembers = {};

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 2, 0);

	PrivKeyShardID shard1 = storage->get_shard_id("avi", usersetID);
	PrivKeyShardID shard2 = storage->get_shard_id("batya", usersetID);

	EXPECT_NE(shard1, shard2);
}

TEST_P(ServerStorageTest, GetShardId_SameUserGetsSameShardForSameUserset)
{
	storage->new_user("avi");

	auto owners = { "avi" };
	std::initializer_list<std::string> regMembers = {};

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 1, 0);

	PrivKeyShardID shard1 = storage->get_shard_id("avi", usersetID);
	PrivKeyShardID shard2 = storage->get_shard_id("avi", usersetID);

	EXPECT_EQ(shard1, shard2);
}

TEST_P(ServerStorageTest, GetShardId_RegularMembersGetShardIDs)
{
	storage->new_user("avi");
	storage->new_user("batya");

	auto owners = { "avi" };
	auto regMembers = { "batya" };

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 1, 1);

	PrivKeyShardID ownerShard = storage->get_shard_id("avi", usersetID);
	PrivKeyShardID memberShard = storage->get_shard_id("batya", usersetID);

	EXPECT_NE(ownerShard, PrivKeyShardID{});
	EXPECT_NE(memberShard, PrivKeyShardID{});
	EXPECT_NE(ownerShard, memberShard);
}

// ----- Integration Tests -----

TEST_P(ServerStorageTest, CompleteWorkflow_CreateUsersUsersetAndVerifyOperations)
{
	// create users
	storage->new_user("avi");
	storage->new_user("batya");
	storage->new_user("gal");

	ASSERT_TRUE(storage->user_exists("avi"));
	ASSERT_TRUE(storage->user_exists("batya"));
	ASSERT_TRUE(storage->user_exists("gal"));

	// create userset
	auto owners = { "avi", "batya" };
	auto regMembers = { "gal" };

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 2, 1);

	// verify userset configuration
	UserSetInfo info = storage->get_userset_info(usersetID);
	EXPECT_EQ(info.owners.size(), 2);
	EXPECT_EQ(info.reg_members.size(), 1);
	EXPECT_EQ(info.owners_threshold, 2);
	EXPECT_EQ(info.reg_members_threshold, 1);

	// verify ownership
	EXPECT_TRUE(storage->user_owns_userset("avi", usersetID));
	EXPECT_TRUE(storage->user_owns_userset("batya", usersetID));
	EXPECT_FALSE(storage->user_owns_userset("gal", usersetID));

	// verify get_usersets
	auto aviSets = storage->get_usersets("avi");
	EXPECT_TRUE(aviSets.contains(usersetID));

	// get shard IDs
	PrivKeyShardID aviShard = storage->get_shard_id("avi", usersetID);
	PrivKeyShardID batyaShard = storage->get_shard_id("batya", usersetID);
	PrivKeyShardID galShard = storage->get_shard_id("gal", usersetID);

	EXPECT_NE(aviShard, batyaShard);
	EXPECT_NE(aviShard, galShard);
	EXPECT_NE(batyaShard, galShard);
}

TEST_P(ServerStorageTest, EdgeCase_EmptyRegularMembersList)
{
	storage->new_user("avi");

	auto owners = { "avi" };
	std::initializer_list<std::string> regMembers = {};

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 1, 0);

	UserSetInfo info = storage->get_userset_info(usersetID);
	EXPECT_EQ(info.owners.size(), 1);
	EXPECT_TRUE(info.reg_members.empty());
}

TEST_P(ServerStorageTest, EdgeCase_ThresholdEqualsGroupSize)
{
	storage->new_user("avi");
	storage->new_user("batya");
	storage->new_user("gal");

	auto owners = { "avi", "batya", "gal" };
	std::initializer_list<std::string> regMembers = {};

	UserSetID usersetID = storage->new_userset(strings(owners), strings(regMembers), 3, 0);

	UserSetInfo info = storage->get_userset_info(usersetID);
	EXPECT_EQ(info.owners_threshold, 3);
	EXPECT_EQ(info.owners.size(), 3);
}

// ===== Instantiation of Parameterized Tests =====

// factory functions for each implementation
static StorageFactory CreateShortTermServerStorage()
{
	return []() -> std::unique_ptr<IServerStorage>
	{
		return std::make_unique<ShortTermServerStorage>();
	};
}

// register the implementations to test
INSTANTIATE_TEST_SUITE_P(
	ServerStorageImplementations,
	ServerStorageTest,
	::testing::Values(
		CreateShortTermServerStorage()
	)
);

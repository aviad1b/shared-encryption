/*********************************************************************
 * \file   test_client_storage.cpp
 * \brief  Contains tests for client (profile) storage.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "../client_api/storage/ProfileStorage.hpp"
#include "../utils/hash.hpp"
#include <gtest/gtest.h>
#include <filesystem>

using senc::clientapi::storage::ProfileRecord;
using senc::clientapi::storage::ProfileStorage;
using senc::utils::ECGroup;

struct ClientStorageTestParams
{
	std::string path;
	std::string username;
	std::string password;
	std::vector<ProfileRecord> records;
};

class ClientStorageTest : public testing::TestWithParam<ClientStorageTestParams>
{
protected:
	std::unique_ptr<ProfileStorage> storage;

	void SetUp() override
	{
		const auto& params = GetParam();
		if (std::filesystem::exists(params.path))
			std::remove(params.path.c_str());
		storage = std::make_unique<ProfileStorage>(
			params.path, params.username, params.password
		);
	}

	void TearDown() override
	{
		storage.reset();
		std::remove(GetParam().path.c_str());
	}
};

TEST_P(ClientStorageTest, WriteReadRoundTrip)
{
	const auto& records = GetParam().records;

	for (const auto& record : records)
		storage->add_profile_data(record);

	auto recordsRange = storage->iter_profile_data();
	std::size_t i = 0;
	for (auto it = recordsRange.begin(); it != recordsRange.end() && i < records.size(); ++it, ++i)
	{
		const ProfileRecord& record = records[i];
		const ProfileRecord& storedRecord = *it;

		EXPECT_EQ(record.is_owner(), storedRecord.is_owner());
		EXPECT_EQ(record.userset_id(), storedRecord.userset_id());
		EXPECT_EQ(record.reg_pub_key(), storedRecord.reg_pub_key());
		EXPECT_EQ(record.owner_pub_key(), storedRecord.owner_pub_key());
		EXPECT_EQ(record.reg_external_priv_key_shard(), storedRecord.reg_external_priv_key_shard());
		if (record.is_owner() && storedRecord.is_owner())
		{
			EXPECT_EQ(record.reg_internal_priv_key_shard(), storedRecord.reg_internal_priv_key_shard());
			EXPECT_EQ(record.owner_external_priv_key_shard(), storedRecord.owner_external_priv_key_shard());
			EXPECT_EQ(record.owner_internal_priv_key_shard(), storedRecord.owner_internal_priv_key_shard());
		}
	}

	EXPECT_EQ(i, records.size());
}

TEST_P(ClientStorageTest, WriteUpdateRead)
{
	const auto& records = GetParam().records;

	for (const auto& record : records)
		storage->add_profile_data(record);

	{
		auto recordsRange = storage->iter_profile_data();
		auto secondIt = recordsRange.begin();
		auto firstIt = secondIt++;

		ProfileRecord profile1 = *firstIt, profile2 = *secondIt;

		// swap profile1 and profile2 using operator*
		*firstIt = profile2;
		*secondIt = profile1;
	}
	
	// test correct values
	{
		std::vector<ProfileRecord> records = GetParam().records;
		std::swap(records[0], records[1]);

		auto recordsRange = storage->iter_profile_data();
		std::size_t i = 0;
		for (auto it = recordsRange.begin(); it != recordsRange.end() && i < records.size(); ++it, ++i)
		{
			const ProfileRecord& record = records[i];
			const ProfileRecord& storedRecord = *it;

			EXPECT_EQ(record.is_owner(), storedRecord.is_owner());
			EXPECT_EQ(record.userset_id(), storedRecord.userset_id());
			EXPECT_EQ(record.reg_pub_key(), storedRecord.reg_pub_key());
			EXPECT_EQ(record.owner_pub_key(), storedRecord.owner_pub_key());
			EXPECT_EQ(record.reg_external_priv_key_shard(), storedRecord.reg_external_priv_key_shard());
			if (record.is_owner() && storedRecord.is_owner())
			{
				EXPECT_EQ(record.reg_internal_priv_key_shard(), storedRecord.reg_internal_priv_key_shard());
				EXPECT_EQ(record.owner_external_priv_key_shard(), storedRecord.owner_external_priv_key_shard());
				EXPECT_EQ(record.owner_internal_priv_key_shard(), storedRecord.owner_internal_priv_key_shard());
			}
		}

		EXPECT_EQ(i, records.size());
	}
}

INSTANTIATE_TEST_SUITE_P(
	ClientStorageTests,
	ClientStorageTest,
	testing::Values(
		ClientStorageTestParams{
			.path = "user.sencp",
			.username = "user",
			.password = "pass123",
			.records = {
				ProfileRecord::owner(
					"57641e16-e02a-473b-8204-a809a9c435df",
					435,
					ECGroup::generator().pow(111),
					ECGroup::generator().pow(222),
					senc::PrivKeyShard{ 3, 333 },
					senc::PrivKeyShard{ 4, 334 },
					senc::PrivKeyShard{ 13, 131313 },
					senc::PrivKeyShard{ 14, 131314 }
				),
				ProfileRecord::reg(
					"51657d81-1d4b-41ca-9749-cd6ee61cc325",
					111,
					ECGroup::generator().pow(435),
					ECGroup::generator().pow(256),
					senc::PrivKeyShard{ 1, 435 }
				),
				ProfileRecord::owner(
					"55b27150-1668-446f-aa50-35d9358eac19",
					283746,
					ECGroup::generator().pow(444),
					ECGroup::generator().pow(555),
					senc::PrivKeyShard{ 4, 666 },
					senc::PrivKeyShard{ 5, 667 },
					senc::PrivKeyShard{ 14, 161616 },
					senc::PrivKeyShard{ 15, 161617 }
				)
			}
		}
	)
);

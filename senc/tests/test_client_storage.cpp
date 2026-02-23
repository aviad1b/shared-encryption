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
        const auto& record = records[i];
        const auto& storedRecord = *it;

        EXPECT_EQ(record.is_owner(), storedRecord.is_owner());
        EXPECT_EQ(record.userset_id(), storedRecord.userset_id());
        EXPECT_EQ(record.reg_layer_pub_key(), storedRecord.reg_layer_pub_key());
        EXPECT_EQ(record.owner_layer_pub_key(), storedRecord.owner_layer_pub_key());
        EXPECT_EQ(record.reg_layer_priv_key_shard(), storedRecord.reg_layer_priv_key_shard());
        if (record.is_owner() && storedRecord.is_owner())
        {
            EXPECT_EQ(record.owner_layer_priv_key_shard(), storedRecord.owner_layer_priv_key_shard());
        }
    }

    EXPECT_EQ(i, records.size());
}

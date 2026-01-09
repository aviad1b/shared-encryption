/*********************************************************************
 * \file   test_threshold_enc.cpp
 * \brief  Contains tests for Shamir threshold encryption utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include <gtest/gtest.h>

#include "../utils/enc/HybridElGamal2L.hpp"
#include "../utils/enc/ECHKDF2L.hpp"
#include "../utils/enc/AES1L.hpp"
#include "../utils/Shamir.hpp"
#include "../utils/bytes.hpp"

using senc::utils::enc::HybridElGamal2L;
using senc::utils::enc::ECHKDF2L;
using senc::utils::enc::AES1L;
using senc::utils::ECGroup;
using senc::utils::Buffer;

struct ThresholdEncTestParams
{
	Buffer data;
	int threshold1;
	int threshold2;
	int numUnits1;
	int numUnits2;
};

struct ThresholdEncTest : testing::Test, testing::WithParamInterface<ThresholdEncTestParams> { };

TEST_P(ThresholdEncTest, ThresholdEnc)
{
	using Shamir = senc::utils::ShamirHybridElGamal<ECGroup, AES1L, ECHKDF2L>;
	using Shard = typename Shamir::Shard;
	using ShardID = typename Shamir::ShardID;
	using Part = typename Shamir::Part;
	HybridElGamal2L<ECGroup, AES1L, ECHKDF2L> schema;
	const Buffer& data = GetParam().data;
	int threshold1 = GetParam().threshold1;
	int threshold2 = GetParam().threshold2;
	int numUnits1 = GetParam().numUnits1;
	int numUnits2 = GetParam().numUnits2;

	const auto [pubKey1, privKey1] = schema.keygen();
	const auto [pubKey2, privKey2] = schema.keygen();

	auto poly1 = Shamir::sample_poly(privKey1, threshold1);
	auto poly2 = Shamir::sample_poly(privKey2, threshold2);

	auto shardsIDs1 = senc::utils::to_vector<ShardID>(std::views::iota(1, numUnits1 + 1));
	auto shardsIDs2 = senc::utils::to_vector<ShardID>(std::views::iota(numUnits1 + 2, numUnits1 + 2 + numUnits2 + 1));

	auto shards1 = Shamir::make_shards(poly1, shardsIDs1);
	auto shards2 = Shamir::make_shards(poly2, shardsIDs2);

	auto encrypted = schema.encrypt(data, pubKey1, pubKey2);
	
	auto parts1 = senc::utils::to_vector<Part>(
		shards1 |
		std::views::transform([&encrypted, &shardsIDs1](const Shard& shard) -> Part
		{
			return Shamir::decrypt_get_2l<1>(encrypted, shard, shardsIDs1);
		})
	);
	auto parts2 = senc::utils::to_vector<Part>(
		shards2 |
		std::views::transform([&encrypted, &shardsIDs2](const Shard& shard) -> Part
		{
			return Shamir::decrypt_get_2l<2>(encrypted, shard, shardsIDs2);
		})
	);

	auto decrypted = Shamir::decrypt_join_2l(encrypted, parts1, parts2);

	EXPECT_EQ(data, decrypted);
}

INSTANTIATE_TEST_SUITE_P(VariousThresholds, ThresholdEncTest, testing::Values(
	// Arbitrary values, exactly above threshold
	ThresholdEncTestParams{ Buffer{0x00, 0x11, 0x22}, 5, 4, 6, 5 },
	ThresholdEncTestParams{ Buffer{0xFF, 0xFF, 0xFF}, 4, 5, 5, 6 },

	// Trivial case: One shard required
	ThresholdEncTestParams{ Buffer{0x00, 0x11, 0x22}, 0, 0, 1, 1 }, // one shard given
	ThresholdEncTestParams{ Buffer{0x00, 0x11, 0x22}, 0, 0, 2, 3 }  // more than one shard given
));

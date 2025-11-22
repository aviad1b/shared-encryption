/*********************************************************************
 * \file   test_shamir.cpp
 * \brief  Contains tests for Shamir secret sharing utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <typeinfo> // needed for intellisense of exception detection

#include "../utils/Random.hpp"
#include "../utils/Shamir.hpp"
#include "../utils/ModInt.hpp"

using senc::utils::IntegralModTraits;
using senc::utils::ShamirException;
using senc::utils::ModInt;
using senc::utils::Random;

using MI7 = ModInt<IntegralModTraits<int, 7, true>>; // prime modulus 7

struct ShareIntTestParams
{
	int secret;
	int threshold;
	int numShards;
	bool success;
};

using ShareModIntTestParams = ShareIntTestParams;

struct ShareIntTest : testing::Test, testing::WithParamInterface<ShareIntTestParams> { };

struct ShareModIntTest : testing::Test, testing::WithParamInterface<ShareModIntTestParams> { };

TEST_P(ShareIntTest, ShareInt)
{
	using Shamir = senc::utils::Shamir<int>;
	auto dist = Random<int>::get_range_dist(-100, 100);
	const auto& params = GetParam();
	auto poly = Shamir::sample_poly(params.secret, params.threshold, dist);
	auto shards = Shamir::make_shards(poly, std::views::iota(1, params.numShards + 1));

	if (params.success)
	{
		auto restored = Shamir::restore_secret(shards, params.threshold);
		EXPECT_EQ(restored, params.secret);
	}
	else
	{
		EXPECT_THROW(Shamir::restore_secret(shards, params.threshold), ShamirException);
	}
}

TEST_P(ShareModIntTest, ShareModInt)
{
	using Shamir = senc::utils::Shamir<MI7>;
	const auto& params = GetParam();
	auto poly = Shamir::sample_poly(params.secret, params.threshold, MI7::sample);
	auto shards = Shamir::make_shards(poly, std::views::iota(1, params.numShards + 1));

	if (params.success)
	{
		auto restored = Shamir::restore_secret(shards, params.threshold);
		EXPECT_EQ(restored, params.secret);
	}
	else
	{
		EXPECT_THROW(Shamir::restore_secret(shards, params.threshold), ShamirException);
	}
}

INSTANTIATE_TEST_CASE_P(ShareInt, ShareIntTest, testing::Values(
	// Basic valid cases
	ShareIntTestParams{ 42, 1, 3, true },      // Simple case: 2-of-3
	ShareIntTestParams{ 0, 1, 2, true },       // Secret is zero
	ShareIntTestParams{ -50, 2, 5, true },     // Negative secret
	ShareIntTestParams{ 100, 0, 1, true },     // Threshold = 0 (trivial case)
	ShareIntTestParams{ 7, 4, 10, true },      // Higher threshold: 5-of-10

	// Edge cases - minimum threshold
	ShareIntTestParams{ 25, 0, 5, true },      // Threshold 1 with multiple shards
	ShareIntTestParams{ -15, 1, 2, true },     // Threshold equals number of shards

	// Edge cases - various shard counts
	ShareIntTestParams{ 99, 2, 3, true },      // Exact threshold match
	ShareIntTestParams{ -99, 3, 8, true },     // 4-of-8
	ShareIntTestParams{ 13, 6, 20, true },     // 7-of-20

	// Failure cases - insufficient shards
	ShareIntTestParams{ 42, 2, 2, false },     // Need 3, only have 2
	ShareIntTestParams{ 10, 4, 4, false },     // Need 5, only have 4
	ShareIntTestParams{ -20, 9, 5, false },    // Need 10, only have 5

	// Boundary values for secrets
	ShareIntTestParams{ -100, 1, 4, true },    // Minimum from distribution
	ShareIntTestParams{ 100, 1, 4, true },     // Maximum from distribution
	ShareIntTestParams{ 1, 1, 2, true },       // Small positive
	ShareIntTestParams{ -1, 1, 2, true }       // Small negative
));

INSTANTIATE_TEST_CASE_P(ShareModInt, ShareModIntTest, testing::Values(
	// Basic valid cases (secrets in range [0, 6])
	ShareModIntTestParams{ 3, 1, 3, true },    // Simple case: 2-of-3
	ShareModIntTestParams{ 0, 1, 2, true },    // Secret is zero
	ShareModIntTestParams{ 6, 2, 5, true },    // Maximum value in field
	ShareModIntTestParams{ 1, 0, 1, true },    // Threshold = 0 (trivial case)
	ShareModIntTestParams{ 5, 3, 6, true },    // Higher threshold: 4-of-6

	// Edge cases - minimum threshold
	ShareModIntTestParams{ 2, 0, 5, true },    // Threshold 0 with multiple shards
	ShareModIntTestParams{ 4, 1, 2, true },    // Threshold equals number of shards

	// Edge cases - various shard counts
	ShareModIntTestParams{ 1, 2, 3, true },    // Exact threshold match
	ShareModIntTestParams{ 6, 3, 6, true },    // 4-of-6 (max shards for mod 7)
	ShareModIntTestParams{ 2, 4, 6, true },    // 5-of-6
	ShareModIntTestParams{ 3, 5, 6, true },    // 6-of-6 (maximum threshold for mod 7)

	// All possible secret values
	ShareModIntTestParams{ 0, 1, 4, true },
	ShareModIntTestParams{ 1, 1, 4, true },
	ShareModIntTestParams{ 2, 1, 4, true },
	ShareModIntTestParams{ 3, 1, 4, true },
	ShareModIntTestParams{ 4, 1, 4, true },
	ShareModIntTestParams{ 5, 1, 4, true },
	ShareModIntTestParams{ 6, 1, 4, true },

	// Failure cases - insufficient shards
	ShareModIntTestParams{ 3, 2, 2, false },   // Need 3, only have 2
	ShareModIntTestParams{ 5, 4, 4, false },   // Need 5, only have 4
	ShareModIntTestParams{ 1, 5, 3, false }    // Need 6, only have 3
));

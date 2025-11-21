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

using senc::utils::ShamirException;
using senc::utils::Random;

struct ShareIntTestParams
{
	int secret;
	int threshold;
	int numShards;
	bool success;
};

struct ShareIntTest : testing::Test, testing::WithParamInterface<ShareIntTestParams> { };

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

INSTANTIATE_TEST_CASE_P(ShareInt, ShareIntTest,
	testing::Values(
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
		ShareIntTestParams{ -20, 9, 5, false },   // Need 10, only have 5

		// Boundary values for secrets
		ShareIntTestParams{ -100, 1, 4, true },    // Minimum from distribution
		ShareIntTestParams{ 100, 1, 4, true },     // Maximum from distribution
		ShareIntTestParams{ 1, 2, 2, true },       // Small positive
		ShareIntTestParams{ -1, 2, 2, true }       // Small negative
	)
);

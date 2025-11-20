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
	auto shards = Shamir::make_shards(poly, { 1, 2, 3 }); // TODO: use actual numShards

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

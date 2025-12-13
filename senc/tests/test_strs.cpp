/*********************************************************************
 * \file   test_strs.cpp
 * \brief  Contains tests for string-related utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include <gtest/gtest.h>

#include "../utils/ECGroup.hpp"
#include "../utils/ModInt.hpp"
#include "../utils/strs.hpp"

using senc::utils::IntegralModTraits;
using senc::utils::ECGroup;
using senc::utils::ModInt;

using senc::utils::from_string;
using senc::utils::to_string;

TEST(Strs, ModInt)
{
	constexpr int modulus = 7;
	using MI = ModInt<IntegralModTraits<int, modulus, true>>;
	for (int i = 0; i < modulus; ++i)
	{
		MI val = i;
		EXPECT_EQ(from_string<MI>(to_string(val)), val);
	}
}

TEST(Strs, ECGroup)
{
	EXPECT_EQ(from_string<ECGroup>(to_string(ECGroup::identity())), ECGroup::identity());
	EXPECT_EQ(from_string<ECGroup>(to_string(ECGroup::generator)), ECGroup::generator());
	for (int i = 0; i < 100; ++i)
	{
		auto elem = ECGroup::sample();
		EXPECT_EQ(from_string<ECGroup>(to_string(elem)), elem);
	}
}

/*********************************************************************
 * \file   test_ranges.cpp
 * \brief  Contains tests for range-related utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <vector>

#include "../utils/ranges.hpp"

using senc::utils::ranges::product;

struct RangeProductTest : testing::Test, testing::WithParamInterface<std::vector<int>> { };

TEST_P(RangeProductTest, Ranges)
{
	const auto& elems = GetParam();
	int prodCalc = 1;
	for (int elem : elems)
		prodCalc *= elem;
	auto prodRet = product(elems);
	EXPECT_TRUE(prodRet.has_value());
	EXPECT_EQ(prodRet.value(), prodCalc);
}

INSTANTIATE_TEST_CASE_P(Ranges, RangeProductTest, testing::Values(
	std::vector<int>({ 1, 1, 1, 1, 1 }),
	std::vector<int>({ 1, 2, 3, 4, 5 }),
	std::vector<int>({ 234, 124, 12 })
));

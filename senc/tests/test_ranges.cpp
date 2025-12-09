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

using senc::utils::views::enumerate;
using senc::utils::views::join;
using senc::utils::views::zip;
using senc::utils::product;
using senc::utils::sum;

struct RangeSumTest : testing::Test, testing::WithParamInterface<std::vector<int>> { };

struct RangeProductTest : testing::Test, testing::WithParamInterface<std::vector<int>> { };

struct RangeEnumerateTest : testing::Test, testing::WithParamInterface<std::vector<int>> { };

TEST_P(RangeSumTest, Ranges)
{
	const auto& elems = GetParam();
	int sumCalc = 0;
	for (int elem : elems)
		sumCalc += elem;
	auto sumRet = sum(elems);
	EXPECT_EQ(sumRet, sumCalc);
}

INSTANTIATE_TEST_CASE_P(Ranges, RangeSumTest, testing::Values(
	std::vector<int>({ 1, 1, 1, 1, 1 }),
	std::vector<int>({ 1, 2, 3, 4, 5 }),
	std::vector<int>({ 234, 124, 12 })
));

TEST_P(RangeProductTest, Ranges)
{
	const auto& elems = GetParam();
	int prodCalc = 1;
	for (int elem : elems)
		prodCalc *= elem;
	auto prodRet = product(elems);
	EXPECT_EQ(prodRet, prodCalc);
}

INSTANTIATE_TEST_CASE_P(Ranges, RangeProductTest, testing::Values(
	std::vector<int>({ 1, 1, 1, 1, 1 }),
	std::vector<int>({ 1, 2, 3, 4, 5 }),
	std::vector<int>({ 234, 124, 12 })
));

TEST_P(RangeEnumerateTest, Ranges)
{
	const auto& elems = GetParam();
	auto it = elems.begin();
	std::size_t i = 0;
	for (auto [idx, elem] : elems | enumerate)
	{
		EXPECT_EQ(*it, elem);
		EXPECT_EQ(i, idx);
		++i;
		++it;
	}
}

INSTANTIATE_TEST_CASE_P(Ranges, RangeEnumerateTest, testing::Values(
	std::vector<int>({ 1, 1, 1, 1, 1 }),
	std::vector<int>({ 1, 2, 3, 4, 5 }),
	std::vector<int>({ 234, 124, 12 })
));

TEST(Ranges, ZipTest)
{
	std::vector<int> x{1, 2, 3, 4, 5};
	std::array<int, 5> y{ 6, 7, 8, 9, 10 };

	for (const auto& [i, ab] : zip(x, y) | enumerate)
	{
		const auto& [a, b] = ab;
		EXPECT_EQ(a, x[i]);
		EXPECT_EQ(b, y[i]);
	}
}

TEST(Ranges, JoinTwoTest)
{
	std::vector<int> x{ 1, 2, 3, 4, 5 };
	std::array<int, 5> y{ 6, 7, 8, 9, 10 };

	std::vector<int> all;
	all.insert(all.end(), x.begin(), x.end());
	all.insert(all.end(), y.begin(), y.end());

	for (const auto& [i, elem] : join(x, y) | enumerate)
		EXPECT_EQ(elem, all[i]);
}

TEST(Ranges, JoinThreeTest)
{
	std::vector<int> x{ 1, 2, 3, 4, 5 };
	std::array<int, 5> y{ 6, 7, 8, 9, 10 };
	std::array<int, 2> z{ -8, 435 };

	std::vector<int> all;
	all.insert(all.end(), x.begin(), x.end());
	all.insert(all.end(), y.begin(), y.end());
	all.insert(all.end(), z.begin(), z.end());

	for (const auto& [i, elem] : join(x, y, z) | enumerate)
		EXPECT_EQ(elem, all[i]);
}

TEST(Ranges, JoinFirstEmpty)
{
	std::vector<std::string> x{};
	std::vector<std::string> y{ "abc", "def" };
	std::vector<std::string> all = y;

	for (const auto& [i, elem] : join(x, y) | enumerate)
		EXPECT_EQ(elem, all[i]);
}

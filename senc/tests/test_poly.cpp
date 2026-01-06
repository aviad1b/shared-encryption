/*********************************************************************
 * \file   test_poly.cpp
 * \brief  Contains tests of the polynom utils (`poly.hpp`).
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../utils/poly.hpp"

using senc::utils::PolyCoeff;
using senc::utils::PolyInput;
using senc::utils::PolyOutput;
using senc::utils::Poly;

template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
struct PolyTestParams
{
	Poly<I, O, C> poly;
	std::vector<C> coeffs;
	std::vector<std::pair<I, O>> expected;

	explicit PolyTestParams(std::vector<C>&& coeffs, std::vector<std::pair<I, O>> expected)
		: poly(std::vector<C>(coeffs)), coeffs(std::move(coeffs)), expected(std::move(expected)) { }
};

using IntPolyTestParams = PolyTestParams<int, int, int>;
struct IntPolyTest : testing::Test, testing::WithParamInterface<IntPolyTestParams> { };

TEST_P(IntPolyTest, Output)
{
	const auto& as = GetParam();
	const auto& poly = as.poly;
	for (const auto& [input, expectedOutput] : as.expected)
		EXPECT_EQ(expectedOutput, poly(input));
}

INSTANTIATE_TEST_SUITE_P(IntegerPolynomials, IntPolyTest, testing::Values(
	IntPolyTestParams
	{
		{ 0, 1 }, // x
		{
			{ 1, 1 },
			{ 2, 2 },
			{ 5, 5 },
			{ 435, 435 },
			{ -9, -9 },
		}
	},
	IntPolyTestParams
	{
		{ 1, 2 }, // 2x + 1
		{
			{ 1, 3 },
			{ 8, 17 },
			{ -9, -17 }
		}
	},
	IntPolyTestParams
	{
		{ 3, -7, 1 }, // x^2-7x+3
		{
			{ 5, -7 },
			{ -9, 147 },
			{ 0, 3 }
		}
	}
));

/*********************************************************************
 * \file   test_fraction.cpp
 * \brief  Contains tests for Fraction class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <typeinfo> // needed for intellisense of exception detection
#include "../utils/Fraction.hpp"
#include "../utils/math.hpp"

using senc::utils::FractionException;
using senc::utils::Fraction;
using senc::utils::pow;

TEST(FractionTests, DefaultCtor)
{
    Fraction<int> f;
    EXPECT_EQ(f.numerator(), 0);
    EXPECT_EQ(f.denominator(), 1);
}

TEST(FractionTests, NumeratorOnlyCtor)
{
    Fraction<int> f(5);
    EXPECT_EQ(f.numerator(), 5);
    EXPECT_EQ(f.denominator(), 1);
}

TEST(FractionTests, NumDenCtorNormalizes)
{
    Fraction<int> f(6, 8); // should normalize to 3/4
    EXPECT_EQ(f.numerator(), 3);
    EXPECT_EQ(f.denominator(), 4);
}

TEST(FractionTests, NegativeSignNormalization)
{
    Fraction<int> f(1, -2); // should normalize to -1/2
    EXPECT_EQ(f.numerator(), -1);
    EXPECT_EQ(f.denominator(), 2);
}

TEST(FractionTests, ThrowsOnZeroDenominator)
{
    EXPECT_THROW(Fraction<int>(7, 0), FractionException);
}

TEST(FractionTests, EqualityAfterNormalization)
{
    Fraction<int> a(2, 4);
    Fraction<int> b(1, 2);
    EXPECT_TRUE(a == b);
}

TEST(FractionTests, Ordering)
{
    Fraction<int> a(1, 3);
    Fraction<int> b(2, 3);
    EXPECT_TRUE((a <=> b) < 0);
    EXPECT_TRUE((b <=> a) > 0);
    EXPECT_TRUE((a <=> a) == 0);
}

TEST(FractionTests, ConvertsToDouble)
{
    Fraction<int> f(1, 2);
    double x = static_cast<double>(f);
    EXPECT_DOUBLE_EQ(x, 0.5);
}

TEST(FractionTests, UnaryMinus)
{
    Fraction<int> f(1, 3);
    auto g = -f;
    EXPECT_EQ(g.numerator(), -1);
    EXPECT_EQ(g.denominator(), 3);
}

TEST(FractionTests, Inverse)
{
    Fraction<int> f(3, 5);
    auto inv = f.inverse();
    EXPECT_EQ(inv.numerator(), 5);
    EXPECT_EQ(inv.denominator(), 3);
}

TEST(FractionTests, InverseThrowsOnZero)
{
    Fraction<int> f(0, 5);
    EXPECT_THROW(f.inverse(), FractionException);
}

TEST(FractionTests, PrefixIncrement)
{
    Fraction<int> f(1, 2);
    ++f; // f = f + 1 = 3/2
    EXPECT_EQ(f.numerator(), 3);
    EXPECT_EQ(f.denominator(), 2);
}

TEST(FractionTests, PostfixIncrement)
{
    Fraction<int> f(1, 2);
    auto old = f++;
    EXPECT_EQ(old.numerator(), 1);
    EXPECT_EQ(old.denominator(), 2);
    EXPECT_EQ(f.numerator(), 3);
    EXPECT_EQ(f.denominator(), 2);
}

TEST(FractionTests, Addition)
{
    Fraction<int> a(1, 2);
    Fraction<int> b(1, 3);
    auto c = a + b;
    EXPECT_EQ(c.numerator(), 5);
    EXPECT_EQ(c.denominator(), 6);
}

TEST(FractionTests, Subtraction)
{
    Fraction<int> a(1, 2);
    Fraction<int> b(1, 3);
    auto c = a - b;
    EXPECT_EQ(c.numerator(), 1);
    EXPECT_EQ(c.denominator(), 6);
}

TEST(FractionTests, Multiplication)
{
    Fraction<int> a(2, 3);
    Fraction<int> b(3, 4);
    auto c = a * b;
    EXPECT_EQ(c.numerator(), 1); // 6/12
    EXPECT_EQ(c.denominator(), 2);
}

TEST(FractionTests, Division)
{
    Fraction<int> a(2, 3);
    Fraction<int> b(3, 4);
    auto c = a / b; // 2/3 * 4/3 = 8/9
    EXPECT_EQ(c.numerator(), 8);
    EXPECT_EQ(c.denominator(), 9);
}

TEST(FractionTests, ZeroDivisionThrows)
{
    Fraction<int> a(5, 7);
    Fraction<int> zero(0, 1);
    EXPECT_THROW(a / zero, FractionException);
}

TEST(FractionTests, Power)
{
    Fraction<int> f(2, 3);
    auto r = f.pow(3); // (2/3)^3 = 8/27
    EXPECT_EQ(r.numerator(), 8);
    EXPECT_EQ(r.denominator(), 27);

    auto r2 = pow(f, 3);
    EXPECT_EQ(r, r2);
}

TEST(FractionTests, PowerZeroGivesOne)
{
    Fraction<int> f(5, 7);
    auto r = f.pow(0);
    EXPECT_EQ(r.numerator(), 1);
    EXPECT_EQ(r.denominator(), 1);

    auto r2 = pow(f, 0);
    EXPECT_EQ(r, r2);
}

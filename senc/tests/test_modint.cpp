/*********************************************************************
 * \file   test_modint.cpp
 * \brief  Contains tests for ModInt class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <typeinfo> // needed for intellisense of exception detection
#include "../utils/ModInt.hpp"
#include "../utils/math.hpp"

using senc::utils::ModException;
using senc::utils::ModInt;

using senc::utils::mod_pow;
using senc::utils::pow;

using MI7 = ModInt<int, 7, true>; // prime modulus 7
using MI6 = ModInt<int, 6, false>; // composite modulus 6

TEST(ModIntTests, BasicCorrectness)
{
    EXPECT_EQ(mod_pow(2, 3, 7), 8 % 7);      // 2^3 = 8 mod 7 -> 1
    EXPECT_EQ(mod_pow(5, 0, 7), 1);          // x^0 = 1 always
    EXPECT_EQ(mod_pow(3, 4, 7), 81 % 7);     // 81 mod 7 = 4
}

TEST(ModIntTests, LargeExponent)
{
    // Fermat: 2^(6 mod 7) mod 7 = 1
    EXPECT_EQ(mod_pow(2, 1000000, 7), mod_pow(2, 1000000 % 6, 7));
}

TEST(ModIntTests, ConstructionAndValue)
{
    MI7 x(10);
    EXPECT_EQ(static_cast<int>(x), 10 % 7); // expect 3
}

TEST(ModIntTests, EqualityComparisons)
{
    MI7 a(3), b(10), c(4);
    EXPECT_TRUE(a == b); // both represent 3
    EXPECT_FALSE(a == c);

    EXPECT_TRUE(3 == a);
    EXPECT_FALSE(4 == a);
}

TEST(ModIntTests, Addition)
{
    MI7 a(5), b(6);
    EXPECT_EQ(static_cast<int>(a + b), (5 + 6) % 7);  // 11 % 7 = 4
    a += b;
    EXPECT_EQ(static_cast<int>(a), 4);
}

TEST(ModIntTests, Subtraction)
{
    MI7 a(1), b(3);
    EXPECT_EQ(static_cast<int>(a - b), (1 - 3 + 7) % 7); // 5
    a -= b;
    EXPECT_EQ(static_cast<int>(a), 5);
}

TEST(ModIntTests, Multiplication)
{
    MI7 a(3), b(4);
    EXPECT_EQ(static_cast<int>(a * b), (3 * 4) % 7); // 12 % 7 = 5
    a *= b;
    EXPECT_EQ(static_cast<int>(a), 5);
}

TEST(ModIntTests, DivisionPrimeModulus)
{
    MI7 a(3), b(5); // inverse of 5 mod 7 is 3
    EXPECT_EQ(static_cast<int>(a / b), (3 * 3) % 7);
}

TEST(ModIntTests, DivisionInvertibleComposite)
{
    // modulus = 6; gcd(5,6) == 1 so 5 is invertible mod 6
    MI6 a(3), b(5);
    // 5^{-1} mod 6 = 5 (because 5*5=25=1 mod 6), so (3/5) = 3*5 = 15 mod 6 = 3
    MI6 c;
    EXPECT_NO_THROW(c = a / b);
    EXPECT_EQ(static_cast<int>(c), 3);
}

TEST(ModIntTests, DivisionThrowsNotInvertible)
{
    // modulus = 6; gcd(2,6) == 2 so 2 has no inverse mod 6
    MI6 a(3), b(2);
    EXPECT_THROW(a / b, ModException);
}

TEST(ModIntTests, Inverse)
{
    MI7 x(3);
    EXPECT_EQ(static_cast<int>(x.inverse()), 5);
}

TEST(ModIntTests, Power)
{
    MI7 x(3);
    EXPECT_EQ(static_cast<int>(x.pow(3)), mod_pow(3, 3, 7));
    EXPECT_EQ(static_cast<int>(pow(x, 3)), mod_pow(3, 3, 7));
}

TEST(ModIntTests, IntPlusModInt)
{
    MI7 x(5);
    EXPECT_EQ(static_cast<int>(2 + x), (2 + 5) % 7);
}

TEST(ModIntTests, IntMinusModInt)
{
    MI7 x(5);
    EXPECT_EQ(static_cast<int>(2 - x), (2 - 5 + 7) % 7);
}

TEST(ModIntTests, IntTimesModInt)
{
    MI7 x(4);
    EXPECT_EQ(static_cast<int>(3 * x), (3 * 4) % 7);
}

TEST(ModIntTests, IntDivModIntPrime)
{
    MI7 x(3); // inverse is 5
    EXPECT_EQ(static_cast<int>(2 / x), (2 * 5) % 7);
}

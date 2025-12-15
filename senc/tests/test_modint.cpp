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

using senc::utils::IntegralModTraits;
using senc::utils::ModException;
using senc::utils::ModInt;
using senc::utils::BigInt;

using senc::utils::mod_pow;
using senc::utils::pow;

using MI7 = ModInt<IntegralModTraits<int, 7, true>>; // prime modulus 7
using MI6 = ModInt<IntegralModTraits<int, 6, false>>; // composite modulus 6

const BigInt CRYPTO_SEVEN = 7;
const BigInt CRYPTO_SIX = 6;

struct CMI7Traits
{
	using Underlying = BigInt;
	static const BigInt& modulus() noexcept { return CRYPTO_SEVEN; }
	static constexpr bool is_known_prime() noexcept { return true; }
};

struct CMI6Traits
{
	using Underlying = BigInt;
	static const BigInt& modulus() noexcept { return CRYPTO_SIX; }
	static constexpr bool is_known_prime() noexcept { return false; }
};

using CMI7 = ModInt<CMI7Traits>;
using CMI6 = ModInt<CMI6Traits>;

TEST(ModIntTests, ModPowBasicCorrectnessWithInt)
{
	EXPECT_EQ(mod_pow(2, 3, 7), 8 % 7);      // 2^3 = 8 mod 7 -> 1
	EXPECT_EQ(mod_pow(5, 0, 7), 1);          // x^0 = 1 always
	EXPECT_EQ(mod_pow(3, 4, 7), 81 % 7);     // 81 mod 7 = 4
}

TEST(ModIntTests, ModPowBasicCorrectnessWithCryptoInt)
{
	EXPECT_EQ(mod_pow(BigInt(2), BigInt(3), BigInt(7)), BigInt(8) % BigInt(7));  // 2^3 = 8 mod 7 -> 1
	EXPECT_EQ(mod_pow(BigInt(5), BigInt(), BigInt(7)), BigInt(1));             // x^0 = 1 always
	EXPECT_EQ(mod_pow(BigInt(3), BigInt(4), BigInt(7)), BigInt(81) % BigInt(7)); // 81 mod 7 = 4
}

TEST(ModIntTests, ModPowHandlesLargeExponentWithInt)
{
	// Fermat: 2^(6 mod 7) mod 7 = 1
	EXPECT_EQ(mod_pow(2, 1000000, 7), mod_pow(2, 1000000 % 6, 7));
}

TEST(ModIntTests, ModPowHandlesLargeExponentWithCryptoInt)
{
	// Fermat: 2^(6 mod 7) mod 7 = 1
	EXPECT_EQ(
		mod_pow(BigInt(2), BigInt(1000000), BigInt(7)),
		mod_pow(BigInt(2), BigInt(1000000) % BigInt(6), BigInt(7))
	);
}

TEST(ModIntTests, ConstructionAndValueWithInt)
{
	MI7 x(10);
	EXPECT_EQ(static_cast<int>(x), 10 % 7); // expect 3
}

TEST(ModIntTests, ConstructionAndValueWithCryptoInt)
{
	CMI7 x(10);
	EXPECT_EQ(static_cast<BigInt>(x), BigInt(10) % BigInt(7)); // expect 3
}

TEST(ModIntTests, EqualityComparisonsWithInt)
{
	MI7 a(3), b(10), c(4);
	EXPECT_TRUE(a == b); // both represent 3
	EXPECT_FALSE(a == c);

	EXPECT_TRUE(3 == a);
	EXPECT_FALSE(4 == a);
}

TEST(ModIntTests, EqualityComparisonsWithCryptoInt)
{
	CMI7 a(3), b(10), c(4);
	EXPECT_TRUE(a == b); // both represent 3
	EXPECT_FALSE(a == c);

	EXPECT_TRUE(3 == a);
	EXPECT_FALSE(4 == a);
}

TEST(ModIntTests, AdditionWithInt)
{
	MI7 a(5), b(6);
	EXPECT_EQ(static_cast<int>(a + b), (5 + 6) % 7);  // 11 % 7 = 4
	a += b;
	EXPECT_EQ(static_cast<int>(a), 4);
}

TEST(ModIntTests, AdditionWithCryptoInt)
{
	CMI7 a(5), b(6);
	EXPECT_EQ(static_cast<BigInt>(a + b), (BigInt(5) + BigInt(6)) % BigInt(7));  // 11 % 7 = 4
	a += b;
	EXPECT_EQ(static_cast<BigInt>(a), BigInt(4));
}

TEST(ModIntTests, SubtractionWithInt)
{
	MI7 a(1), b(3);
	EXPECT_EQ(static_cast<int>(a - b), (1 - 3 + 7) % 7); // 5
	a -= b;
	EXPECT_EQ(static_cast<int>(a), 5);
}

TEST(ModIntTests, SubtractionWithCryptoInt)
{
	CMI7 a(1), b(3);
	EXPECT_EQ(static_cast<BigInt>(a - b), (BigInt(1) - BigInt(3) + BigInt(7)) % BigInt(7)); // 5
	a -= b;
	EXPECT_EQ(static_cast<BigInt>(a), BigInt(5));
}

TEST(ModIntTests, MultiplicationWithInt)
{
	MI7 a(3), b(4);
	EXPECT_EQ(static_cast<int>(a * b), (3 * 4) % 7); // 12 % 7 = 5
	a *= b;
	EXPECT_EQ(static_cast<int>(a), 5);
}

TEST(ModIntTests, MultiplicationWithCryptoInt)
{
	CMI7 a(3), b(4);
	EXPECT_EQ(static_cast<BigInt>(a * b), (BigInt(3) * BigInt(4)) % BigInt(7)); // 12 % 7 = 5
	a *= b;
	EXPECT_EQ(static_cast<BigInt>(a), BigInt(5));
}

TEST(ModIntTests, DivisionPrimeModulusWithInt)
{
	MI7 a(3), b(5); // inverse of 5 mod 7 is 3
	EXPECT_EQ(static_cast<int>(a / b), (3 * 3) % 7);
}

TEST(ModIntTests, DivisionPrimeModulusWithCryptoInt)
{
	CMI7 a(3), b(5); // inverse of 5 mod 7 is 3
	EXPECT_EQ(static_cast<BigInt>(a / b), (BigInt(3) * BigInt(3)) % BigInt(7));
}

TEST(ModIntTests, DivisionInvertibleCompositeWithInt)
{
	// modulus = 6; gcd(5,6) == 1 so 5 is invertible mod 6
	MI6 a(3), b(5);
	// 5^{-1} mod 6 = 5 (because 5*5=25=1 mod 6), so (3/5) = 3*5 = 15 mod 6 = 3
	MI6 c;
	EXPECT_NO_THROW(c = a / b);
	EXPECT_EQ(static_cast<int>(c), 3);
}

TEST(ModIntTests, DivisionInvertibleCompositeWithCryptoInt)
{
	// modulus = 6; gcd(5,6) == 1 so 5 is invertible mod 6
	CMI6 a(3), b(5);
	// 5^{-1} mod 6 = 5 (because 5*5=25=1 mod 6), so (3/5) = 3*5 = 15 mod 6 = 3
	CMI6 c;
	EXPECT_NO_THROW(c = a / b);
	EXPECT_EQ(static_cast<BigInt>(c), BigInt(3));
}

TEST(ModIntTests, DivisionThrowsNotInvertibleWithInt)
{
	// modulus = 6; gcd(2,6) == 2 so 2 has no inverse mod 6
	MI6 a(3), b(2);
	EXPECT_THROW(a / b, ModException);
}

TEST(ModIntTests, DivisionThrowsNotInvertibleWithCryptoInt)
{
	// modulus = 6; gcd(2,6) == 2 so 2 has no inverse mod 6
	CMI6 a(3), b(2);
	EXPECT_THROW(a / b, ModException);
}

TEST(ModIntTests, InverseWithInt)
{
	MI7 x(3);
	EXPECT_EQ(static_cast<int>(x.inverse()), 5);
}

TEST(ModIntTests, InverseWithCryptoInt)
{
	CMI7 x(3);
	EXPECT_EQ(static_cast<BigInt>(x.inverse()), BigInt(5));
}

TEST(ModIntTests, PowerWithInt)
{
	MI7 x(3);
	EXPECT_EQ(static_cast<int>(x.pow(3)), mod_pow(3, 3, 7));
	EXPECT_EQ(static_cast<int>(pow(x, 3)), mod_pow(3, 3, 7));
}

TEST(ModIntTests, PowerWithCryptoInt)
{
	CMI7 x(3);
	EXPECT_EQ(static_cast<BigInt>(x.pow(BigInt(3))), mod_pow(BigInt(3), BigInt(3), BigInt(7)));
	EXPECT_EQ(static_cast<BigInt>(pow(x, BigInt(3))), mod_pow(BigInt(3), BigInt(3), BigInt(7)));
}

TEST(ModIntTests, IntPlusModIntWithInt)
{
	MI7 x(5);
	EXPECT_EQ(static_cast<int>(2 + x), (2 + 5) % 7);
}

TEST(ModIntTests, IntPlusModIntWithCryptoInt)
{
	CMI7 x(5);
	EXPECT_EQ(static_cast<BigInt>(BigInt(2) + x), (BigInt(2) + BigInt(5)) % BigInt(7));
}

TEST(ModIntTests, IntMinusModIntWithInt)
{
	MI7 x(5);
	EXPECT_EQ(static_cast<int>(2 - x), (2 - 5 + 7) % 7);
}

TEST(ModIntTests, IntMinusModIntWithCryptoInt)
{
	CMI7 x(5);
	EXPECT_EQ(static_cast<BigInt>(BigInt(2) - x), (BigInt(2) - BigInt(5) + BigInt(7)) % BigInt(7));
}

TEST(ModIntTests, IntTimesModIntWithInt)
{
	MI7 x(4);
	EXPECT_EQ(static_cast<int>(3 * x), (3 * 4) % 7);
}

TEST(ModIntTests, IntTimesModIntWithCryptoInt)
{
	CMI7 x(4);
	EXPECT_EQ(static_cast<BigInt>(BigInt(3) * x), (BigInt(3) * BigInt(4)) % BigInt(7));
}

TEST(ModIntTests, IntDivModIntPrimeWithInt)
{
	MI7 x(3); // inverse is 5
	EXPECT_EQ(static_cast<int>(2 / x), (2 * 5) % 7);
}

TEST(ModIntTests, IntDivModIntPrimeWithCryptoInt)
{
	CMI7 x(3); // inverse is 5
	EXPECT_EQ(static_cast<BigInt>(BigInt(2) / x), (BigInt(2) * BigInt(5)) % BigInt(7));
}

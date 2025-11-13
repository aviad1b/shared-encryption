/*********************************************************************
 * \file   ModInt_impl.hpp
 * \brief  Contains implementation of modular int class.
 *
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "ModInt.hpp"

#include <sstream>

namespace senc::utils
{
	template <typename T, typename E>
	T mod_pow(T base, E exp, const T& modulus) SENC_REQ_NOEXCEPT(
		(Copyable, T),
		(Copyable, E),
		(IntConstructible, T),
		(SelfModulable, T),
		(LowerComparable, E),
		(Andable, E),
		(SelfRightShiftable, E)
	)
	{
		// fast exponent algorithm under prime modulus
		T res = 1;
		base %= modulus;
		while (0 < exp)
		{
			if (exp & 1)
				res = (res * base) % modulus;
			base = (base * base) % modulus;
			exp >>= 1;
		}
		return res;
	}

	template <typename T>
	T prime_modular_inverse(T value, const T& modulus) SENC_REQ_NOEXCEPT(
		(Copyable, T),
		(IntConstructible, T),
		(SelfModulable, T),
		(LowerComparable, T),
		(Andable, T),
		(SelfRightShiftable, T)
	)
	{
		// using Fermat's formula for prime modulus
		return mod_pow(value, modulus - 2, modulus);
	}

	template <typename T>
	T modular_inverse(const T& value, const T& modulus) SENC_REQ(
		(Copyable, T),
		(IntConstructible, T),
		(EqualityComparable, T),
		(Addable, T),
		(Subtractable, T),
		(Multiplicable, T),
		(Devisible, T),
		(Modulable, T)
	)
	{
		// using the extended euclidean algorithm

		T a = value, b = modulus;
		T x0 = 1, x1 = 0;

		while (b != 0)
		{
			T q = a / b;
			T temp = a % b;
			a = b;
			b = temp;

			temp = x0 - q * x1;
			x0 = x1;
			x1 = temp;
		}

		// now a = gcd(value, modulus). if gcd != 1, inverse doesn't exist
		if (1 != a)
		{
			if constexpr (Outputable<T>)
			{
				std::stringstream s;
				s << "No inverse for " << value << " under modulus " << modulus;
				throw ModException(s.str());
			}
			else throw ModException("No inverse for given value under given modulus");
		}

		// x0 may be negative, fix it
		return (x0 % modulus + modulus) % modulus;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	const Distribution<Int> ModInt<Int, modulus, isPrime>::DIST = Random<Int>::get_dist_below(modulus);

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::ModInt()
		noexcept(IntConstructibleNoExcept<Int>) : Self(0) { }

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::ModInt(Int value)
		noexcept(ModulableNoExcept<Int> && CopyableNoExcept<Int>)
		: _value(value % modulus) { }

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::sample()
		noexcept(UnderlyingDistTypeNoExcept<UnderlyingDist<Int>, Int> &&
			ModulableNoExcept<Int> && CopyableNoExcept<Int>)
	requires DistVal<Int>
	{
		return Self(DIST());
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::operator const Int&() const noexcept
	{
		return this->_value;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline bool ModInt<Int, modulus, isPrime>::operator==(Self other) const SENC_REQ_NOEXCEPT(
		(EqualityComparable, Int)
	)
	{
		return (this->_value == other._value);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline bool ModInt<Int, modulus, isPrime>::operator==(Int value) const SENC_REQ_NOEXCEPT(
		(EqualityComparable, Int)
	)
	{
		return (this->_value == value);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator-() const SENC_REQ_NOEXCEPT_COND(
		ModulableNoExcept<Int> && CopyableNoExcept<Int>,
		(Subtractable, Int)
	)
	{
		return Self(modulus - this->_value);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::inverse() const SENC_REQ_NOEXCEPT_COND(
		(isPrime && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(LowerComparable, Int),
		(Andable, Int),
		(SelfRightShiftable, Int)
	)
	{
		if constexpr (isPrime)
			return prime_modular_inverse(this->_value, modulus);
		else
			return modular_inverse(this->_value, modulus);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator++() SENC_REQ_NOEXCEPT_COND(
		SelfModulableNoExcept<Int>,
		(LeftIncrementable, Int)
	)
	{
		++this->_value;
		this->_value %= modulus;
		return *this;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator++(int) SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(RightIncrementable, Int)
	)
	{
		return this->_value++; // ctor automatically applies modulo
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator+(Int value) const SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Addable, Int)
	)
	{
		return Self(this->_value + value);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator+=(Int value) SENC_REQ_NOEXCEPT_COND(
		SelfModulableNoExcept<Int>,
		(SelfAddable, Int)
	)
	{
		this->_value += value;
		this->_value %= modulus;
		return *this;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator+(Self other) const SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Addable, Int)
	)
	{
		return *this + other._value;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator+=(Self other) SENC_REQ_NOEXCEPT_COND(
		SelfModulableNoExcept<Int>,
		(SelfAddable, Int)
	)
	{
		return *this += other._value;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator--() SENC_REQ_NOEXCEPT_COND(
		CopyableNoExcept<Int>,
		(LeftDecrementable, Int),
		(EqualityComparable, Int)
	)
	{
		if (this->_value == 0)
			this->_value = modulus;
		--this->_value;
		return *this;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator--(int) SENC_REQ_NOEXCEPT_COND(
		CopyableNoExcept<Int>,
		(RightDecrementable, Int),
		(EqualityComparable, Int)
	)
	{
		if (this->_value == 0)
		{
			// if zero, set to max then return zero
			this->_value = modulus;
			this->_value--;
			return 0;
		}
		return this->_value--; // ctor automatically applies modulo
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator-(Int value) const SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Addable, Int),
		(Subtractable, Int)
	)
	{
		return Self((modulus + this->_value - value) % modulus);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator-=(Int value) SENC_REQ_NOEXCEPT_COND(
		SelfModulableNoExcept<Int>,
		(SelfAddable, Int),
		(SelfSubtractable, Int)
	)
	{
		this->_value += modulus;
		this->_value -= value;
		this->_value %= modulus;
		return *this;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator-(Self other) const SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Addable, Int),
		(Subtractable, Int)
	)
	{
		return *this - other._value;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator-=(Self other) SENC_REQ_NOEXCEPT_COND(
		SelfModulableNoExcept<Int>,
		(SelfAddable, Int),
		(SelfSubtractable, Int)
	)
	{
		return *this -= other._value;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator*(Int value) const SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Multiplicable, Int)
	)
	{
		return Self((this->_value * value) % modulus);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator*=(Int value) SENC_REQ_NOEXCEPT_COND(
		SelfModulableNoExcept<Int>,
		(SelfMultiplicable, Int)
	)
	{
		this->_value *= value;
		this->_value %= modulus;
		return *this;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator*(Self other) const SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Multiplicable, Int)
	)
	{
		return *this * other._value;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator*=(Self other) SENC_REQ_NOEXCEPT_COND(
		SelfModulableNoExcept<Int>,
		(SelfMultiplicable, Int)
	)
	{
		return *this *= other._value;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator/(Int value) const SENC_REQ_NOEXCEPT_COND(
		(isPrime && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Multiplicable, Int),
		(LowerComparable, Int),
		(Andable, Int),
		(SelfRightShiftable, Int)
	)
	{
		return *this / Self(value);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator/=(Int value) SENC_REQ_NOEXCEPT_COND(
		(isPrime && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Multiplicable, Int),
		(LowerComparable, Int),
		(Andable, Int),
		(SelfRightShiftable, Int)
	)
	{
		return *this = *this / value;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator/(Self other) const SENC_REQ_NOEXCEPT_COND(
		(isPrime && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Multiplicable, Int),
		(LowerComparable, Int),
		(Andable, Int),
		(SelfRightShiftable, Int)
	)
	{
		return *this * other.inverse();
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator/=(Self other) SENC_REQ_NOEXCEPT_COND(
		(isPrime && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Multiplicable, Int),
		(LowerComparable, Int),
		(Andable, Int),
		(SelfRightShiftable, Int)
	)
	{
		return *this = *this / other;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	template <typename Exp>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::pow(const Exp& exp) SENC_REQ_NOEXCEPT_COND(
		(CopyableNoExcept<Int> && SelfModulableNoExcept<Int> && IntConstructibleNoExcept<Int>),
		(LowerComparable, Exp),
		(Andable, Exp),
		(SelfRightShiftable, Exp)
	)
	{
		return Self(mod_pow(this->_value, exp, modulus));
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline bool operator==(Int value, ModInt<Int, modulus, isPrime> modint) SENC_REQ_NOEXCEPT(
		(EqualityComparable, Int)
	)
	{
		return (modint == value);
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator+(Int a, ModInt<Int, modulus, isPrime> b) SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Addable, Int)
	)
	{
		return b + a;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime> operator-(Int a, ModInt<Int, modulus, isPrime> b) SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Addable, Int),
		(Subtractable, Int)
	)
	{
		return ModInt<Int, modulus, isPrime>(a % modulus) - b;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime> operator*(Int a, ModInt<Int, modulus, isPrime> b) SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Multiplicable, Int)
	)
	{
		return b * a;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime> operator/(Int a, ModInt<Int, modulus, isPrime> b) SENC_REQ_NOEXCEPT_COND(
		(isPrime && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
		(Multiplicable, Int),
		(LowerComparable, Int),
		(Andable, Int),
		(SelfRightShiftable, Int)
	)
	{
		return ModInt<Int, modulus, isPrime>(a % modulus) / b;
	}

	template <ModIntUnderlyingType Int, Int modulus, bool isPrime>
	inline std::ostream& operator<<(std::ostream& os, ModInt<Int, modulus, isPrime> modint)
	requires Outputable<Int>
	{
		return os << (Int)modint;
	}
}

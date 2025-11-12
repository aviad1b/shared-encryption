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
	T mod_pow(T base, E exp, T modulus) SENC_REQ(
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
	T prime_modular_inverse(T value, T modulus) SENC_REQ(
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

	template <std::integral T>
	T modular_inverse(T value, T modulus)
	{
		// using the extended euclidean algorithm

		T a = value, b = modulus;
		T x0 = 1, x1 = 0;

		while (b)
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
			std::stringstream s;
			s << "No inverse for " << value << " under modulus " << modulus;
			throw ModException(s.str());
		}

		// x0 may be negative, fix it
		return (x0 % modulus + modulus) % modulus;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::ModInt() noexcept : Self(0) { }

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::ModInt(Int value) noexcept : _value(value % modulus) { }

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::sample() noexcept
	{
		return Self(DIST());
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::operator Int() const noexcept
	{
		return this->_value;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline bool ModInt<Int, modulus, isPrime>::operator==(Self other) const noexcept
	{
		return (this->_value == other._value);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline bool ModInt<Int, modulus, isPrime>::operator==(Int value) const noexcept
	{
		return (this->_value == value);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	const Distribution<Int> ModInt<Int, modulus, isPrime>::DIST = Random<Int>::get_dist_below(modulus);

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator-() const noexcept
	{
		return Self(modulus - this->_value);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::inverse() const noexcept(isPrime)
	{
		if constexpr (isPrime)
			return prime_modular_inverse(this->_value, modulus);
		else
			return modular_inverse(this->_value, modulus);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator++() noexcept
	{
		this->_value++;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator++(int) noexcept
	{
		Self res = *this;
		++(*this);
		return res;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator+(Int value) const noexcept
	{
		return Self((this->_value + value) % modulus);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator+=(Int value) noexcept
	{
		this->_value += value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator+(Self other) const noexcept
	{
		return *this + other._value;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator+=(Self other) noexcept
	{
		return *this += other._value;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator--() noexcept
	{
		// not safe to use `--` here directly since we may go below zero
		return *this -= 1;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator--(int) noexcept
	{
		Self res = *this;
		--(*this);
		return res;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator-(Int value) const noexcept
	{
		return Self((modulus + this->_value - value) % modulus);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator-=(Int value) noexcept
	{
		this->_value += modulus;
		this->_value -= value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator-(Self other) const noexcept
	{
		return *this - other._value;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator-=(Self other) noexcept
	{
		return *this -= other._value;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator*(Int value) const noexcept
	{
		return Self((this->_value * value) % modulus);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator*=(Int value) noexcept
	{
		this->_value *= value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator*(Self other) const noexcept
	{
		return *this * other._value;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator*=(Self other) noexcept
	{
		return *this *= other._value;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator/(Int value) const noexcept(isPrime)
	{
		return *this / Self(value % modulus);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self& ModInt<Int, modulus, isPrime>::operator/=(Int value) noexcept(isPrime)
	{
		return *this = *this / value;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator/(Self other) const noexcept(isPrime)
	{
		return *this * other.inverse();
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::operator/=(Self other) noexcept(isPrime)
	{
		return *this = *this / other;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	inline ModInt<Int, modulus, isPrime>::Self ModInt<Int, modulus, isPrime>::pow(std::integral auto exp) noexcept
	{
		return Self(mod_pow(this->_value, exp, modulus));
	}

	template <std::integral Int, Int modulus, bool isPrime>
	bool operator==(Int value, ModInt<Int, modulus, isPrime> modint) noexcept
	{
		return (modint == value);
	}

	template <std::integral Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator+(Int a, ModInt<Int, modulus, isPrime> b) noexcept
	{
		return b + a;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator-(Int a, ModInt<Int, modulus, isPrime> b) noexcept
	{
		return ModInt<Int, modulus, isPrime>(a % modulus) - b;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator*(Int a, ModInt<Int, modulus, isPrime> b) noexcept
	{
		return b * a;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator/(Int a, ModInt<Int, modulus, isPrime> b) noexcept(isPrime)
	{
		return ModInt<Int, modulus, isPrime>(a % modulus) / b;
	}

	template <std::integral Int, Int modulus, bool isPrime>
	std::ostream& operator<<(std::ostream& os, ModInt<Int, modulus, isPrime> modint)
	{
		return os << (Int)modint;
	}
}

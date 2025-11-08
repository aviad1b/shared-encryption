/*********************************************************************
 * \file   ModInt_impl.hpp
 * \brief  Contains implementation of modular int class.
 *
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "ModInt.hpp"

namespace senc::utils
{
	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::ModInt() noexcept : Self(0) { }

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::ModInt(Int value) noexcept : _value(value % modulus) { }

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::sample() noexcept
	{
		return Self(DIST());
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::operator Int() const noexcept
	{
		return this->_value;
	}

	template <std::integral Int, Int modulus>
	inline bool ModInt<Int, modulus>::operator==(Self other) const noexcept
	{
		return (this->_value == other._value);
	}

	template <std::integral Int, Int modulus>
	inline bool ModInt<Int, modulus>::operator==(Int value) const noexcept
	{
		return (this->_value == value);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator-() const noexcept
	{
		return Self(modulus - this->_value);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::inverse() const
	{
		return Self(modular_inverse(this->_value, modulus));
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator++() noexcept
	{
		this->_value++;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator++(int) noexcept
	{
		Self res = *this;
		++(*this);
		return res;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator+(Int value) const noexcept
	{
		return Self((this->_value + value) % modulus);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator+=(Int value) noexcept
	{
		this->_value += value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator+(Self other) const noexcept
	{
		return *this + other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator+=(Self other) noexcept
	{
		return *this += other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator--() noexcept
	{
		// not safe to use `--` here directly since we may go below zero
		return *this -= 1;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator++(int) noexcept
	{
		Self res = *this;
		--(*this);
		return res;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator-(Int value) const noexcept
	{
		Self((modulus + *this->_value - value) % modulus);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator-=(Int value) noexcept
	{
		this->_value += modulus;
		this->_value -= value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator-(Self other) const noexcept
	{
		return *this - other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator-=(Self other) noexcept
	{
		return *this -= other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator*(Int value) const noexcept
	{
		return Self((this->_value * value) % modulus);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator*=(Int value) noexcept
	{
		this->_value *= value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator*(Self other) const noexcept
	{
		return *this + other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator*=(Self other) noexcept
	{
		return *this += other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator/(Int value) const
	{
		return *this / Self(value % modulus);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator/=(Int value)
	{
		return *this = *this / value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator/(Self other) const
	{
		return *this * other.inverse();
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator/=(Self other)
	{
		return *this = *this / other;
	}

	template <std::integral Int, Int modulus>
	bool operator==(Int value, ModInt<Int, modulus> modint) noexcept
	{
		return (modint == value);
	}

	template <std::integral Int, Int modulus>
	ModInt<Int, modulus> operator+(Int a, ModInt<Int, modulus> b) noexcept
	{
		return b + a;
	}

	template <std::integral Int, Int modulus>
	ModInt<Int, modulus> operator-(Int a, ModInt<Int, modulus> b) noexcept
	{
		return ModInt<Int, modulus>(a % modulus) - b;
	}

	template <std::integral Int, Int modulus>
	ModInt<Int, modulus> operator*(Int a, ModInt<Int, modulus> b) noexcept
	{
		return b * a;
	}

	template <std::integral Int, Int modulus>
	ModInt<Int, modulus> operator/(Int a, ModInt<Int, modulus> b)
	{
		return ModInt<Int, modulus>(a % modulus) / b;
	}

	template <std::integral Int, Int modulus>
	std::ostream& operator<<(std::ostream& os, ModInt<Int, modulus> modint)
	{
		return os << (Int)modint;
	}
}

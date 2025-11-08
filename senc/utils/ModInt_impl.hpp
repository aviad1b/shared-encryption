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
	inline ModInt<Int, modulus>::ModInt() : Self(0) { }

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::ModInt(Int value) : _value(value % modulus) { }

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::sample()
	{
		return Self(DIST());
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::operator Int() const
	{
		return this->_value;
	}

	template <std::integral Int, Int modulus>
	inline bool ModInt<Int, modulus>::operator==(Self other) const
	{
		return (this->_value == other._value);
	}

	template <std::integral Int, Int modulus>
	inline bool ModInt<Int, modulus>::operator==(Int value) const
	{
		return (this->_value == value);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator-() const
	{
		return Self(modulus - this->_value);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator++()
	{
		this->_value++;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator++(int)
	{
		Self res = *this;
		++(*this);
		return res;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator+(Int value) const
	{
		return Self((this->_value + value) % modulus);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator+=(Int value)
	{
		this->_value += value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator+(Self other) const
	{
		return *this + other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator+=(Self other)
	{
		return *this += other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator--()
	{
		// not safe to use `--` here directly since we may go below zero
		return *this -= 1;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator++(int)
	{
		Self res = *this;
		--(*this);
		return res;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator-(Int value) const
	{
		Self((modulus + *this->_value - value) % modulus);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator-=(Int value)
	{
		this->_value += modulus;
		this->_value -= value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator-(Self other) const
	{
		return *this - other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator-=(Self other)
	{
		return *this -= other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator*(Int value) const
	{
		return Self((this->_value * value) % modulus);
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator*=(Int value)
	{
		this->_value *= value;
		this->_value %= modulus;
		return *this;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self ModInt<Int, modulus>::operator*(Self other) const
	{
		return *this + other._value;
	}

	template <std::integral Int, Int modulus>
	inline ModInt<Int, modulus>::Self& ModInt<Int, modulus>::operator*=(Self other)
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
	bool operator==(Int value, ModInt<Int, modulus> modint)
	{
		return (modint == value);
	}

	template <std::integral Int, Int modulus>
	ModInt<Int, modulus> operator+(Int a, ModInt<Int, modulus> b)
	{
		return b + a;
	}

	template <std::integral Int, Int modulus>
	ModInt<Int, modulus> operator-(Int a, ModInt<Int, modulus> b)
	{
		return ModInt<Int, modulus>(a % modulus) - b;
	}

	template <std::integral Int, Int modulus>
	ModInt<Int, modulus> operator*(Int a, ModInt<Int, modulus> b)
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

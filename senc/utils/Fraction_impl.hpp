/*********************************************************************
 * \file   Fraction_impl.hpp
 * \brief  Implementation of `Fraction` class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5781
 *********************************************************************/

#include "Fraction.hpp"

namespace senc::utils
{
	template <std::integral Int>
	inline Fraction<Int>::Fraction() noexcept : Self(0) { }
	
	template <std::integral Int>
	inline Fraction<Int>::Fraction(Int numerator, Int denominator)
		: _num(numerator), _den(denominator)
	{
		if (!this->_den)
			throw FractionException("Denominator can't be zero");
		normalize();
	}

	template <std::integral Int>
	inline Int Fraction<Int>::numerator() const noexcept
	{
		return this->_num;
	}

	template <std::integral Int>
	inline Int Fraction<Int>::denominator() const noexcept
	{
		return this->_den;
	}

	template <std::integral Int>
	inline std::strong_ordering Fraction<Int>::operator<=>(const Self& other) const noexcept
	{
		const Int left = this->_num * other._den;
		const Int right = other._num * this->_den;

		return left <=> right;
	}

	template <std::integral Int>
	inline bool Fraction<Int>::operator==(const Self& other) const noexcept
	{
		return this->_num == other._num && this->_den == other._den;
	}

	template <std::integral Int>
	inline Fraction<Int>::operator double() const noexcept
	{
		return static_cast<double>(this->_num) / static_cast<double>(this->_den);
	}

	template <std::integral Int>
	inline Fraction<Int>::Self Fraction<Int>::operator-() const noexcept
	{
		return Self(-this->_num, this->_den);
	}

	template <std::integral Int>
	inline Fraction<Int>::Self Fraction<Int>::inverse() const
	{
		return Self(this->_den, this->_num);
	}

	template <std::integral Int>
	inline Fraction<Int>::Self& Fraction<Int>::operator++() noexcept
	{
		return *this += 1;
	}

	template <std::integral Int>
	inline Fraction<Int>::Self Fraction<Int>::operator++(int) noexcept
	{
		Self res = *this;
		++(*this);
		return res;
	}

	template <std::integral Int>
	inline Fraction<Int>::Self Fraction<Int>::operator+(const Self& other) const noexcept
	{
		return Self(this->_num * other._den + other._num * this->_den, this->_den * other._den);
	}

	template <std::integral Int>
	inline Fraction<Int>::Self& Fraction<Int>::operator+=(const Self& other) noexcept
	{
		return *this = *this + other;
	}

	template <std::integral Int>
	inline Fraction<Int>::Self& Fraction<Int>::operator--() noexcept
	{
		return *this -= 1;
	}

	template <std::integral Int>
	inline Fraction<Int>::Self Fraction<Int>::operator--(int) noexcept
	{
		Self res = *this;
		--(*this);
		return res;
	}

	template <std::integral Int>
	inline Fraction<Int>::Self Fraction<Int>::operator-(const Self& other) const noexcept
	{
		return *this + (-other);
	}

	template <std::integral Int>
	inline Fraction<Int>::Self& Fraction<Int>::operator-=(const Self& other) noexcept
	{
		return *this = *this - other;
	}

	template <std::integral Int>
	inline Fraction<Int>::Self Fraction<Int>::operator*(const Self& other) const noexcept
	{
		return Self(this->_num * other._num, this->_den * other._den);
	}

	template <std::integral Int>
	inline Fraction<Int>::Self& Fraction<Int>::operator*=(const Self& other) noexcept
	{
		return *this = *this * other;
	}

	template <std::integral Int>
	inline Fraction<Int>::Self Fraction<Int>::operator/(const Self& other) const
	{
		return *this * other.inverse();
	}

	template <std::integral Int>
	inline Fraction<Int>::Self& Fraction<Int>::operator/=(const Self& other)
	{
		return *this = *this / other;
	}

	template <std::integral Int>
	inline void Fraction<Int>::normalize() noexcept
	{
		// move sign to numerator
		if (this->_den < 0)
		{
			this->_den = -this->_den;
			this->_num = -this->_num;
		}

		long long g = std::gcd(this->_num, this->_den);
		this->_num /= g;
		this->_den /= g;
	}

	template <std::integral Int>
	std::ostream& operator<<(std::ostream& os, const Fraction<Int>& frac)
	{
		return os << frac.numerator() << "/" << frac.denominator();
	}
}

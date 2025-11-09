/*********************************************************************
 * \file   Fraction.hpp
 * \brief  Header of `Fraction` class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <concepts>
#include <compare>
#include <ostream>

namespace senc::utils
{
	/**
	 * @class senc::utils::Fraction
	 * @brief Represents a fraction.
	 * @tparam Int Base integer type (for numerator and denominator).
	 */
	template <std::integral Int>
	class Fraction
	{
	public:
		using Self = Fraction<Int>;

		Fraction();

		Fraction(Int numerator, Int denominator = 1);

		Fraction(const Self&) = default;

		Self& operator=(const Self&) = default;

		Fraction(Self&&) = default;

		Self& operator(Self&&) = default;

		Int numerator() const;

		Int denominator() const;

		std::strong_ordering operator<=>(const Self& other) const;

		bool operator==(const Self& other) const;

		explicit operator double() const;

		Self operator-() const;

		Self inverse() const;

		Self operator+(const Self& other) const;

		Self& operator+=(const Self& other);

		Self operator-(const Self& other) const;

		Self& operator-=(const Self& other);

		Self operator*(const Self& other) const;

		Self& operator*=(const Self& other);

		Self operator/(const Self& other) const;

		Self& operator/=(const Self& other);

	private:
		Int _num, _den;

		void normalize();
	};

	template <std::integral Int>
	std::ostream& operator<<(std::ostream& os, const Fraction<Int>& frac);

	template <std::integral Int>
	std::ostream& operator<<(std::ostream& os, const Fraction<Int>& frac)
	{
		return os << frac.numerator() << "/" << frac.denominator();
	}
}

#include "Fraction_impl.hpp"

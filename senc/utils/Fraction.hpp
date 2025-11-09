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
#include "Exception.hpp"

namespace senc::utils
{
	/**
	 * @class senc::utils::FractionException
	 * @brief Type of exceptions thrown on fraction computation errors.
	 */
	class FractionException : public Exception
	{
	public:
		using Self = FractionException;
		using Base = Exception;

		FractionException(const std::string& msg);

		FractionException(std::string&& msg);

		FractionException(const std::string& msg, const std::string& info);

		FractionException(std::string&& msg, const std::string& info);

		FractionException(const Self&) = default;

		Self& operator=(const Self&) = default;

		FractionException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};

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

		Fraction() noexcept;

		Fraction(Int numerator, Int denominator = 1);

		Fraction(const Self&) noexcept = default;

		Self& operator=(const Self&) noexcept = default;

		Fraction(Self&&) noexcept = default;

		Self& operator(Self&&) noexcept = default;

		Int numerator() noexcept const;

		Int denominator() noexcept const;

		std::strong_ordering operator<=>(const Self& other) const noexcept;

		bool operator==(const Self& other) const noexcept;

		explicit operator double() const noexcept;

		Self operator-() const noexcept;

		Self inverse() const;

		Self operator+(const Self& other) const noexcept;

		Self& operator+=(const Self& other) noexcept;

		Self operator-(const Self& other) const noexcept;

		Self& operator-=(const Self& other) noexcept;

		Self operator*(const Self& other) const noexcept;

		Self& operator*=(const Self& other) noexcept;

		Self operator/(const Self& other) const;

		Self& operator/=(const Self& other);

	private:
		Int _num, _den;

		void normalize() noexcept;
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

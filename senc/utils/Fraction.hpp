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

		/**
		 * @brief Constructs a zero-value fraction.
		 */
		Fraction() noexcept;

		/**
		 * @brief Constructs a (normalized) fraction for given numerator and denominator.
		 * @param numerator Numerator for fraction.
		 * @param denominator Denominator for fraction.
		 * @throw FractionException If attemps to divide by zero.
		 */
		Fraction(Int numerator, Int denominator = 1);

		/**
		 * @brief Copy constructor of fraction.
		 */
		Fraction(const Self&) noexcept = default;

		/**
		 * @brief Copy assignment operator of fraction.
		 */
		Self& operator=(const Self&) noexcept = default;

		/**
		 * @brief Move constructor of fraction.
		 */
		Fraction(Self&&) noexcept = default;

		/**
		 * @brief Move assignment operator of fraction.
		 */
		Self& operator(Self&&) noexcept = default;

		/**
		 * @brief Gets (normalized) fraction numenator.
		 * @return Numenator.
		 */
		Int numerator() const noexcept;

		/**
		 * @brief Gets (normalized) fraction denominator.
		 * @return Denominator.
		 */
		Int denominator() const noexcept;

		/**
		 * @brief Compares the fraction with another.
		 * @param other Other fraction to compare to.
		 * @return Ordering between `*this` and `other`.
		 */
		std::strong_ordering operator<=>(const Self& other) const noexcept;

		/**
		 * @brief Compares the fraction with another.
		 * @param other Other fraction to compare to.
		 * @return `true` if `*this` is the same fraction as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const noexcept;

		/**
		 * @brief Explicit conversion between fraction and `double`.
		 */
		explicit operator double() const noexcept;

		/**
		 * @brief Negates fraction.
		 * @return Negative of `*this`.
		 */
		Self operator-() const noexcept;

		/**
		 * @brief Gets inverse of fraction.
		 * @return Inverse of `*this`.
		 * @throw FractionException If attemps to divide by zero.
		 */
		Self inverse() const;

		/**
		 * @brief Adds another fraction with this.
		 * @param other Other fraction to add with this.
		 * @return Addition result.
		 */
		Self operator+(const Self& other) const noexcept;

		/**
		 * @brief Adds another fraction to this.
		 * @param other Other fraction to add to this.
		 * @return `*this`, after addition.
		 */
		Self& operator+=(const Self& other) noexcept;

		/**
		 * @brief Subtracts another fraction with this.
		 * @param other Other fraction to subtract with this.
		 * @return Subtraction result.
		 */
		Self operator-(const Self& other) const noexcept;

		/**
		 * @brief Subtracts another fraction from this.
		 * @param other Other fraction to subtract from this.
		 * @return `*this`, after subtraction.
		 */
		Self& operator-=(const Self& other) noexcept;

		/**
		 * @brief Multiplies another fraction with this.
		 * @param other Other fraction to multiply with this.
		 * @return Multiplication result.
		 */
		Self operator*(const Self& other) const noexcept;

		/**
		 * @brief Multiplies fraction by another.
		 * @param other Other fraction to multiply this by.
		 * @return `*this`, after multiplication.
		 */
		Self& operator*=(const Self& other) noexcept;

		/**
		 * @brief Divides another fraction with this.
		 * @param other Other fraction to divide with this.
		 * @return Division result.
		 * @throw FractionException If attemps to divide by zero.
		 */
		Self operator/(const Self& other) const;

		/**
		 * @brief Divides this fraction by another.
		 * @param other Other fraction to divide this by.
		 * @return `*this`, after division.
		 * @throw FractionException If attemps to divide by zero.
		 */
		Self& operator/=(const Self& other);

	private:
		Int _num, _den;

		/**
		 * @brief Gets fraction to its normalized form.
		 */
		void normalize() noexcept;
	};

	/**
	 * @brief Fraction output operator.
	 */
	template <std::integral Int>
	std::ostream& operator<<(std::ostream& os, const Fraction<Int>& frac);
}

#include "Fraction_impl.hpp"

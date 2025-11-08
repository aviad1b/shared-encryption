/*********************************************************************
 * \file   ModInt.hpp
 * \brief  Contains declaration of modular int class.
 *
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <concepts>
#include <ostream>

namespace senc::utils
{
	/**
	 * @class senc::utils::ModInt
	 * @brief Modular integer type.
	 * @tparam Int Integer type to base modular integer on.
	 * @tparam modulus Modulus for integer computations.
	 */
	template <std::integral Int, Int modulus>
	class ModInt
	{
	public:
		using Self = ModInt<Int, modulus>;
		constexpr Int MODULUS = modulus;

		/**
		 * @brief Constructs a modular integer with zero value.
		 */
		ModInt();

		/**
		 * @brief Constructs a modular integer with a given value.
		 */
		ModInt(Int value);

		/**
		 * @brief Samples a random modular integer.
		 * @return Sampled modular integer.
		 */
		static Self sample();

		/**
		 * @brief Copy constructor of modular integer.
		 */
		ModInt(const Self&) = default;

		/**
		 * @brief Copy assignment operator of modular integer.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Move constructor of modular integer.
		 */
		ModInt(Self&&) = default;

		/**
		 * @brief Move assignment operator of modular integer.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Casting of modular integer into its underlying fundamental.
		 */
		operator Int() const;

		/**
		 * @brief Checks if the modular integer is equal to another.
		 * @param other Other modular integer to compare to.
		 * @return `true` if `*this` has same value as `other`, otherwise `false`.
		 */
		bool operator==(Self other) const;

		/**
		 * @brief Checks if the modular integer has a given value.
		 * @param value Value to check if the modular integer has.
		 * @return `true` if `*this` has value `value`, othewise `false`.
		 */
		bool operator==(Int value) const;

		/**
		 * @brief Adds another modular integer with this one.
		 * @param other Other modular integer to add with this one.
		 * @return Addition result.
		 */
		Self operator+(Self other) const;

		/**
		 * @brief Adds another modular integer to this one.
		 * @param other Other modular integer to add to this one.
		 * @return `*this`, after addition.
		 */
		Self& operator+=(Self other);

		/**
		 * @brief Multiplies another modular integer with this one.
		 * @param other Other modular integer to multiply with this one.
		 * @return Multiplication result.
		 */
		Self operator*(Self other) const;

		/**
		 * @brief Multiplies the modular integer with another.
		 * @param other Other modular integer to multiply this one by.
		 * @return `*this`, after multiplication.
		 */
		Self& operator*=(Self other);

		/**
		 * @brief Divides the modular integer by another.
		 * @param other Other modular integer to divide this one with.
		 * @return Division result.
		 */
		Self operator/(Self other) const;

		/**
		 * @brief Divides the modular integer by another.
		 * @param other Other modular integer to divide this one by.
		 * @return `*this`, after division.
		 */
		Self operator/=(Self other);

	private:
		Int _value;
	};

	/**
	 * @brief Checks if a modular integer has a given value.
	 * @param value Value to check if the modular integer has.
	 * @param modint Modular integer to check if has value `value`.
	 * @return `true` if `modint` has value `value`, othewise `false`.
	 */
	template <std::integral Int, Int modulus>
	bool operator==(Int value, ModInt<Int, modulus> modint);

	template <std::integral Int, Int modulus>
	std::ostream& operator<<(std::ostream& os, ModInt<Int, modulus> modint);
}

#include "ModInt_impl.hpp"

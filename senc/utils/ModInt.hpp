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
#include "Exception.hpp"
#include "Random.hpp"

namespace senc::utils
{
	/**
	 * @class senc::utils::ModException
	 * @brief Type of exceptions thrown on modular computation errors.
	 */
	class ModException : public Exception
	{
	public:
		using Self = ModException;
		using Base = Exception;

		ModException(const std::string& msg);

		ModException(std::string&& msg);

		ModException(const std::string& msg, const std::string& info);

		ModException(std::string&& msg, const std::string& info);

		ModException(const Self&) = default;

		Self& operator=(const Self&) = default;

		ModException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};

	/**
	 * @brief Raises given value to a given power under a given modulus.
	 * @param base Value to raise to power.
	 * @param exp Exponent to raise `base` by.
	 * @param modulus Modulus to compute power under.
	 * @return `base` raised to the power of `exp` under modulus `modulus`.
	 */
	template <std::integral T, std::integral E>
	T mod_pow(T base, E exp, T modulus) noexcept;

	/**
	 * @brief Finds modular inverse of a given value under given prime modulus.
	 * @param value Value to find modular inverse of.
	 * @param moduls Modulus to find inverse of `value` under.
	 * @return Modular inverse of `value` under `modulus`.
	 */
	template <std::integral T>
	T prime_modular_inverse(T value, T modulus);

	/**
	 * @brief Finds modular inverse of a given value under given modulus.
	 * @param value Value to find modular inverse of.
	 * @param moduls Modulus to find inverse of `value` under.
	 * @return Modular inverse of `value` under `modulus`.
	 * @throw ModException If failed to find inverse.
	 */
	template <std::integral T>
	T modular_inverse(T value, T modulus);

	/**
	 * @class senc::utils::ModInt
	 * @brief Modular integer type.
	 * @tparam Int Integer type to base modular integer on.
	 * @tparam modulus Modulus for integer computations.
	 * @tparam isPrime `true` if `modulus` is known to be prime (for more efficient computations).
	 */
	template <std::integral Int, Int modulus, bool isPrime = false>
	class ModInt
	{
	public:
		using Self = ModInt<Int, modulus, isPrime>;
		static constexpr Int MODULUS = modulus;

		/**
		 * @brief Constructs a modular integer with zero value.
		 */
		ModInt() noexcept;

		/**
		 * @brief Constructs a modular integer with a given value.
		 */
		ModInt(Int value) noexcept;

		/**
		 * @brief Samples a random modular integer.
		 * @return Sampled modular integer.
		 */
		static Self sample() noexcept;

		/**
		 * @brief Copy constructor of modular integer.
		 */
		ModInt(const Self&) noexcept = default;

		/**
		 * @brief Copy assignment operator of modular integer.
		 */
		Self& operator=(const Self&) noexcept = default;

		/**
		 * @brief Move constructor of modular integer.
		 */
		ModInt(Self&&) noexcept = default;

		/**
		 * @brief Move assignment operator of modular integer.
		 */
		Self& operator=(Self&&) noexcept = default;

		/**
		 * @brief Casting of modular integer into its underlying fundamental.
		 */
		operator Int() const noexcept;

		/**
		 * @brief Checks if the modular integer is equal to another.
		 * @param other Other modular integer to compare to.
		 * @return `true` if `*this` has same value as `other`, otherwise `false`.
		 */
		bool operator==(Self other) const noexcept;

		/**
		 * @brief Checks if the modular integer has a given value.
		 * @param value Value to check if the modular integer has.
		 * @return `true` if `*this` has value `value`, othewise `false`.
		 */
		bool operator==(Int value) const noexcept;

		/**
		 * @brief Negates modular integer (under modulus).
		 * @return Negative of `*this` under modulus.
		 */
		Self operator-() const noexcept;

		/**
		 * @brief Gets inverse of modular integer.
		 * @return Modular inverse.
		 * @throw ModException If failed to find inverse (only if `isPrime` is not `true`).
		 */
		Self inverse() const noexcept(isPrime);

		/**
		 * @brief Increases modular integer.
		 * @return `*this` after being increased.
		 */
		Self& operator++() noexcept;

		/**
		 * @brief Increases modular integer.
		 * @return `*this` before being increased.
		 */
		Self operator++(int) noexcept;

		/**
		 * @brief Adds integer value with this modular integer.
		 * @param value Integer value to add with this.
		 * @return Addition result.
		 */
		Self operator+(Int value) const noexcept;

		/**
		 * @brief Adds integer value to this modular integer.
		 * @param value Integer value to add to this.
		 * @return `*this`, after addition.
		 */
		Self& operator+=(Int value) noexcept;

		/**
		 * @brief Adds another modular integer with this one.
		 * @param other Other modular integer to add with this one.
		 * @return Addition result.
		 */
		Self operator+(Self other) const noexcept;

		/**
		 * @brief Adds another modular integer to this one.
		 * @param other Other modular integer to add to this one.
		 * @return `*this`, after addition.
		 */
		Self& operator+=(Self other) noexcept;

		/**
		 * @brief Decreases modular integer.
		 * @return `*this` after being decreased.
		 */
		Self& operator--() noexcept;

		/**
		 * @brief Decreases modular integer.
		 * @return `*this` before being decreased.
		 */
		Self operator--(int) noexcept;

		/**
		 * @brief Subtracts the modular integer by an integer value.
		 * @param value Integer value to subtract this with.
		 * @return Subtraction result.
		 */
		Self operator-(Int value) const noexcept;

		/**
		 * @brief Subtracts the modular integer by an integer value.
		 * @param value Integer value to subtract this by.
		 * @return `*this`, after subtraction.
		 */
		Self& operator-=(Int value) noexcept;

		/**
		 * @brief Subtract the modular integer by another.
		 * @param other Other modular integer to subtract this one with.
		 * @return Subtraction result.
		 */
		Self operator-(Self other) const noexcept;

		/**
		 * @brief Subtracts the modular integer by another.
		 * @param other Other modular integer to subtract this one by.
		 * @return `*this`, after subtraction.
		 */
		Self& operator-=(Self other) noexcept;

		/**
		 * @brief Multiplies integer value with this modular integer.
		 * @param value Integer value to multiply with this.
		 * @return Multiplication result.
		 */
		Self operator*(Int value) const noexcept;

		/**
		 * @brief Multiplies the modular integer with a given integer value.
		 * @param value Integer value to multiply this by.
		 * @return `*this`, after multiplication.
		 */
		Self& operator*=(Int value) noexcept;

		/**
		 * @brief Multiplies another modular integer with this one.
		 * @param other Other modular integer to multiply with this one.
		 * @return Multiplication result.
		 */
		Self operator*(Self other) const noexcept;

		/**
		 * @brief Multiplies the modular integer with another.
		 * @param other Other modular integer to multiply this one by.
		 * @return `*this`, after multiplication.
		 */
		Self& operator*=(Self other) noexcept;

		/**
		 * @brief Divides the modular integer by an integer value.
		 * @param value Integer value to divide this with.
		 * @return Division result.
		 * @throw ModException If failed to divide (only if `isPrime` is not `true`).
		 */
		Self operator/(Int value) const noexcept(isPrime);

		/**
		 * @brief Divides the modular integer by an integer value.
		 * @param value Integer value to divide this by.
		 * @return `*this`, after division.
		 * @throw ModException If failed to divide (only if `isPrime` is not `true`).
		 */
		Self& operator/=(Int value) noexcept(isPrime);

		/**
		 * @brief Divides the modular integer by another.
		 * @param other Other modular integer to divide this one with.
		 * @return Division result.
		 * @throw ModException If failed to divide (only if `isPrime` is not `true`).
		 */
		Self operator/(Self other) const noexcept(isPrime);

		/**
		 * @brief Divides the modular integer by another.
		 * @param other Other modular integer to divide this one by.
		 * @return `*this`, after division.
		 * @throw ModException If failed to divide (only if `isPrime` is not `true`).
		 */
		Self operator/=(Self other) noexcept(isPrime);

		/**
		 * @brief Raises modular integer to given power.
		 * @param exp Exponent to raise `*this` to.
		 * @return Raised modular inetger.
		 */
		Self pow(std::integral auto exp) noexcept;

	private:
		static const Distribution<Int> DIST = Random<Int>::get_dist_below(modulus);

		Int _value;
	};

	/**
	 * @brief Checks if a modular integer has a given value.
	 * @param value Value to check if the modular integer has.
	 * @param modint Modular integer to check if has value `value`.
	 * @return `true` if `modint` has value `value`, othewise `false`.
	 */
	template <std::integral Int, Int modulus, bool isPrime>
	bool operator==(Int value, ModInt<Int, modulus, isPrime> modint) noexcept;

	/**
	 * @brief Adds an integer value with a modular integer.
	 * @param a Integer value.
	 * @param b Modular integer.
	 * @return Addition result.
	 */
	template <std::integral Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator+(Int a, ModInt<Int, modulus, isPrime> b) noexcept;

	/**
	 * @brief Subtracts an integer value from a modular integer.
	 * @param a Integer value.
	 * @param b Modular integer.
	 * @return Subtraction result.
	 */
	template <std::integral Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator-(Int a, ModInt<Int, modulus, isPrime> b) noexcept;

	/**
	 * @brief Multiplies an integer value with a modular integer.
	 * @param a Integer value.
	 * @param b Modular integer.
	 * @return Multiplication result.
	 */
	template <std::integral Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator*(Int a, ModInt<Int, modulus, isPrime> b) noexcept;

	/**
	 * @brief Divides an integer value by a modular integer.
	 * @param a Integer value.
	 * @param b Modular integer.
	 * @return Division result.
	 * @throw ModException If failed to divide (only if `isPrime` is not `true`).
	 */
	template <std::integral Int, Int modulus, bool isPrime>
	ModInt<Int, modulus, isPrime> operator/(Int a, ModInt<Int, modulus, isPrime> b) noexcept(isPrime);

	template <std::integral Int, Int modulus, bool isPrime>
	std::ostream& operator<<(std::ostream& os, ModInt<Int, modulus, isPrime> modint);
}

#include "ModInt_impl.hpp"

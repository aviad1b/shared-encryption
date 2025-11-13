/*********************************************************************
 * \file   ModInt.hpp
 * \brief  Contains declaration of modular int class.
 *
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <ostream>
#include "Exception.hpp"
#include "concepts.hpp"
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
	 * @param exp None-negative exponent to raise `base` by.
	 * @param modulus Modulus to compute power under.
	 * @return `base` raised to the power of `exp` under modulus `modulus`.
	 */
	template <typename T, typename E>
	T mod_pow(T base, E exp, const T& modulus) SENC_REQ_NOEXCEPT(
		(Copyable, T),
		(Copyable, E),
		(IntConstructible, T),
		(SelfModulable, T),
		(LowerComparable, E),
		(Andable, E),
		(SelfDevisible, E)
	);

	/**
	 * @brief Finds modular inverse of a given value under given prime modulus.
	 * @param value Value to find modular inverse of.
	 * @param moduls Modulus to find inverse of `value` under.
	 * @return Modular inverse of `value` under `modulus`.
	 */
	template <typename T>
	T prime_modular_inverse(T value, const T& modulus) SENC_REQ_NOEXCEPT(
		(Copyable, T),
		(IntConstructible, T),
		(SelfModulable, T),
		(LowerComparable, T),
		(Andable, T),
		(SelfDevisible, T)
	);

	/**
	 * @brief Finds modular inverse of a given value under given modulus.
	 * @param value Value to find modular inverse of.
	 * @param moduls Modulus to find inverse of `value` under.
	 * @return Modular inverse of `value` under `modulus`.
	 * @throw ModException If failed to find inverse.
	 */
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
	);

	/**
	 * @concept senc::utils::ModIntUnderlyingType
	 * @brief Looks for a typename that can be used as an underlying type for ModInt.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ModIntUnderlyingType = Copyable<Self> &&
		Modulable<Self> && SelfModulable<Self> && IntConstructible<Self>;

	/**
	 * @concept senc::utils::ModTraitsType
	 * @brief Looks for a typename which defines traits for ModInt.
	 *		  Requires traits:
	 *        - Underlying type (`Underlying`)
	 *        - Modulus (`modulus()`)
	 *        - Is known prime (`constexpr is_known_prime()`).
	 * @note A concept cannot really check for that, but `modulus()` should be constant.
	 */
	template <typename Self>
	concept ModTraitsType = ModIntUnderlyingType<typename Self::Underlying> && requires
	{
		{ Self::modulus() } noexcept -> ConvertibleToNoExcept<const typename Self::Underlying&>;
		{ Self::is_known_prime() } noexcept -> BoolConvertibleNoExcept;
		{ []() constexpr { return Self::is_known_prime(); }() }; // must be constexpr-evaluable
	};

	/**
	 * @struct senc::utils::IntegralModTraits
	 * @brief A `ModTraitsType` for integral-based ModInt.
	 * @tparam Int Underlying integral type for ModInt.
	 * @tparam mod Modulus value.
	 * @tparam isKnownPrime Whether or not `mod` is a known prime.
	 */
	template <std::integral Int, Int mod, bool isKnownPrime = false>
	struct IntegralModTraits
	{
		using Underlying = Int;
		static constexpr Int modulus() noexcept { return mod; }
		static constexpr bool is_known_prime() noexcept { return isKnownPrime; }
	};

	/**
	 * @class senc::utils::ModInt
	 * @brief Modular integer type.
	 * @tparam ModTraits Modulo traits (must satisfy `ModTraitsType`).
	 */
	template <ModTraitsType ModTraits>
	class ModInt
	{
	public:
		using Self = ModInt<ModTraits>;
		using Int = typename ModTraits::Underlying;
		static constexpr bool IS_PRIME_MOD = ModTraits::is_known_prime();
		static auto modulus() { return ModTraits::modulus(); }

		/**
		 * @brief Constructs a modular integer with zero value.
		 */
		ModInt() noexcept(DefaultConstructibleNoExcept<Int>)
		requires (DefaultConstructible<Int> && !std::is_fundamental_v<Int>);

		/**
		 * @brief Constructs a modular integer with zero value.
		 */
		ModInt() noexcept(ZeroConstructibleNoExcept<Int>)
		requires (ZeroConstructible<Int> && (!DefaultConstructible<Int> || std::is_fundamental_v<Int>));

		/**
		 * @brief Constructs a modular integer with a given value.
		 */
		ModInt(Int value) noexcept(ModulableNoExcept<Int> && CopyableNoExcept<Int>);

		/**
		 * @brief Samples a random modular integer.
		 * @return Sampled modular integer.
		 */
		static Self sample()
			noexcept(UnderlyingDistTypeNoExcept<UnderlyingDist<Int>, Int> &&
				ModulableNoExcept<Int> && CopyableNoExcept<Int>)
		requires DistVal<Int>;

		/**
		 * @brief Copy constructor of modular integer.
		 */
		ModInt(const Self&) noexcept(std::is_nothrow_copy_constructible_v<Int>) = default;

		/**
		 * @brief Copy assignment operator of modular integer.
		 */
		Self& operator=(const Self&) noexcept(std::is_nothrow_copy_assignable_v<Int>) = default;

		/**
		 * @brief Move constructor of modular integer.
		 */
		ModInt(Self&&) noexcept(std::is_nothrow_move_constructible_v<Int>) = default;

		/**
		 * @brief Move assignment operator of modular integer.
		 */
		Self& operator=(Self&&) noexcept(std::is_nothrow_move_assignable_v<Int>) = default;

		/**
		 * @brief Casting of modular integer into its underlying fundamental.
		 */
		operator const Int&() const noexcept;

		/**
		 * @brief Checks if the modular integer is equal to another.
		 * @param other Other modular integer to compare to.
		 * @return `true` if `*this` has same value as `other`, otherwise `false`.
		 */
		bool operator==(Self other) const SENC_REQ_NOEXCEPT(
			(EqualityComparable, Int)
		);

		/**
		 * @brief Checks if the modular integer has a given value.
		 * @param value Value to check if the modular integer has.
		 * @return `true` if `*this` has value `value`, othewise `false`.
		 */
		bool operator==(Int value) const SENC_REQ_NOEXCEPT(
			(EqualityComparable, Int)
		);

		/**
		 * @brief Negates modular integer (under modulus).
		 * @return Negative of `*this` under modulus.
		 */
		Self operator-() const SENC_REQ_NOEXCEPT_COND(
			ModulableNoExcept<Int> && CopyableNoExcept<Int>,
			(Subtractable, Int)
		);

		/**
		 * @brief Gets inverse of modular integer.
		 * @return Modular inverse.
		 * @throw ModException If failed to find inverse (only if modulus is not known to be prime).
		 */
		Self inverse() const SENC_REQ_NOEXCEPT_COND(
			(IS_PRIME_MOD && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(LowerComparable, Int),
			(Andable, Int),
			(SelfDevisible, Int)
		);

		/**
		 * @brief Increases modular integer.
		 * @return `*this` after being increased.
		 */
		Self& operator++() SENC_REQ_NOEXCEPT_COND(
			SelfModulableNoExcept<Int>,
			(LeftIncrementable, Int)
		);

		/**
		 * @brief Increases modular integer.
		 * @return `*this` before being increased.
		 */
		Self operator++(int) SENC_REQ_NOEXCEPT_COND(
			(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(RightIncrementable, Int)
		);

		/**
		 * @brief Adds integer value with this modular integer.
		 * @param value Integer value to add with this.
		 * @return Addition result.
		 */
		Self operator+(Int value) const SENC_REQ_NOEXCEPT_COND(
			(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Addable, Int)
		);

		/**
		 * @brief Adds integer value to this modular integer.
		 * @param value Integer value to add to this.
		 * @return `*this`, after addition.
		 */
		Self& operator+=(Int value) SENC_REQ_NOEXCEPT_COND(
			SelfModulableNoExcept<Int>,
			(SelfAddable, Int)
		);

		/**
		 * @brief Adds another modular integer with this one.
		 * @param other Other modular integer to add with this one.
		 * @return Addition result.
		 */
		Self operator+(Self other) const SENC_REQ_NOEXCEPT_COND(
			(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Addable, Int)
		);

		/**
		 * @brief Adds another modular integer to this one.
		 * @param other Other modular integer to add to this one.
		 * @return `*this`, after addition.
		 */
		Self& operator+=(Self other) SENC_REQ_NOEXCEPT_COND(
			SelfModulableNoExcept<Int>,
			(SelfAddable, Int)
		);

		/**
		 * @brief Decreases modular integer.
		 * @return `*this` after being decreased.
		 */
		Self& operator--() SENC_REQ_NOEXCEPT_COND(
			CopyableNoExcept<Int>,
			(LeftDecrementable, Int),
			(EqualityComparable, Int)
		);

		/**
		 * @brief Decreases modular integer.
		 * @return `*this` before being decreased.
		 */
		Self operator--(int) SENC_REQ_NOEXCEPT_COND(
			CopyableNoExcept<Int>,
			(RightDecrementable, Int),
			(EqualityComparable, Int)
		);

		/**
		 * @brief Subtracts the modular integer by an integer value.
		 * @param value Integer value to subtract this with.
		 * @return Subtraction result.
		 */
		Self operator-(Int value) const SENC_REQ_NOEXCEPT_COND(
			(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Addable, Int),
			(Subtractable, Int)
		);

		/**
		 * @brief Subtracts the modular integer by an integer value.
		 * @param value Integer value to subtract this by.
		 * @return `*this`, after subtraction.
		 */
		Self& operator-=(Int value) SENC_REQ_NOEXCEPT_COND(
			SelfModulableNoExcept<Int>,
			(SelfAddable, Int),
			(SelfSubtractable, Int)
		);

		/**
		 * @brief Subtract the modular integer by another.
		 * @param other Other modular integer to subtract this one with.
		 * @return Subtraction result.
		 */
		Self operator-(Self other) const SENC_REQ_NOEXCEPT_COND(
			(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Addable, Int),
			(Subtractable, Int)
		);

		/**
		 * @brief Subtracts the modular integer by another.
		 * @param other Other modular integer to subtract this one by.
		 * @return `*this`, after subtraction.
		 */
		Self& operator-=(Self other) SENC_REQ_NOEXCEPT_COND(
			SelfModulableNoExcept<Int>,
			(SelfAddable, Int),
			(SelfSubtractable, Int)
		);

		/**
		 * @brief Multiplies integer value with this modular integer.
		 * @param value Integer value to multiply with this.
		 * @return Multiplication result.
		 */
		Self operator*(Int value) const SENC_REQ_NOEXCEPT_COND(
			(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Multiplicable, Int)
		);

		/**
		 * @brief Multiplies the modular integer with a given integer value.
		 * @param value Integer value to multiply this by.
		 * @return `*this`, after multiplication.
		 */
		Self& operator*=(Int value) SENC_REQ_NOEXCEPT_COND(
			SelfModulableNoExcept<Int>,
			(SelfMultiplicable, Int)
		);

		/**
		 * @brief Multiplies another modular integer with this one.
		 * @param other Other modular integer to multiply with this one.
		 * @return Multiplication result.
		 */
		Self operator*(Self other) const SENC_REQ_NOEXCEPT_COND(
			(ModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Multiplicable, Int)
		);

		/**
		 * @brief Multiplies the modular integer with another.
		 * @param other Other modular integer to multiply this one by.
		 * @return `*this`, after multiplication.
		 */
		Self& operator*=(Self other) SENC_REQ_NOEXCEPT_COND(
			SelfModulableNoExcept<Int>,
			(SelfMultiplicable, Int)
		);

		/**
		 * @brief Divides the modular integer by an integer value.
		 * @param value Integer value to divide this with.
		 * @return Division result.
		 * @throw ModException If failed to divide (only if modulus is not known to be prime).
		 */
		Self operator/(Int value) const SENC_REQ_NOEXCEPT_COND(
			(IS_PRIME_MOD && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Multiplicable, Int),
			(LowerComparable, Int),
			(Andable, Int),
			(SelfDevisible, Int)
		);

		/**
		 * @brief Divides the modular integer by an integer value.
		 * @param value Integer value to divide this by.
		 * @return `*this`, after division.
		 * @throw ModException If failed to divide (only if modulus is not known to be prime).
		 */
		Self& operator/=(Int value) SENC_REQ_NOEXCEPT_COND(
			(IS_PRIME_MOD && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Multiplicable, Int),
			(LowerComparable, Int),
			(Andable, Int),
			(SelfDevisible, Int)
		);

		/**
		 * @brief Divides the modular integer by another.
		 * @param other Other modular integer to divide this one with.
		 * @return Division result.
		 * @throw ModException If failed to divide (only if modulus is not known to be prime).
		 */
		Self operator/(Self other) const SENC_REQ_NOEXCEPT_COND(
			(IS_PRIME_MOD && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Multiplicable, Int),
			(LowerComparable, Int),
			(Andable, Int),
			(SelfDevisible, Int)
		);

		/**
		 * @brief Divides the modular integer by another.
		 * @param other Other modular integer to divide this one by.
		 * @return `*this`, after division.
		 * @throw ModException If failed to divide (only if modulus is not known to be prime).
		 */
		Self operator/=(Self other) SENC_REQ_NOEXCEPT_COND(
			(IS_PRIME_MOD && ModulableNoExcept<Int> && SelfModulableNoExcept<Int> && CopyableNoExcept<Int>),
			(Multiplicable, Int),
			(LowerComparable, Int),
			(Andable, Int),
			(SelfDevisible, Int)
		);

		/**
		 * @brief Raises modular integer to given power.
		 * @param exp Non-negative exponent to raise `*this` to.
		 * @return Raised modular inetger.
		 */
		template <typename Exp>
		Self pow(const Exp& exp) SENC_REQ_NOEXCEPT_COND(
			(CopyableNoExcept<Int> && SelfModulableNoExcept<Int> && IntConstructibleNoExcept<Int>),
			(LowerComparable, Exp),
			(Andable, Exp),
			(SelfDevisible, Exp)
		);

	private:
		static inline const Distribution<Int> DIST = Random<Int>::get_dist_below(modulus());

		Int _value;
	};

	/**
	 * @brief Checks if a modular integer has a given value.
	 * @param value Value to check if the modular integer has.
	 * @param modint Modular integer to check if has value `value`.
	 * @return `true` if `modint` has value `value`, othewise `false`.
	 */
	template <ModTraitsType ModTraits>
	bool operator==(const typename ModInt<ModTraits>::Int& value, const ModInt<ModTraits>& modint) SENC_REQ_NOEXCEPT(
		(EqualityComparable, typename ModInt<ModTraits>::Int)
	);

	/**
	 * @brief Adds an integer value with a modular integer.
	 * @param a Integer value.
	 * @param b Modular integer.
	 * @return Addition result.
	 */
	template <ModTraitsType ModTraits>
	inline ModInt<ModTraits> operator+(const typename ModInt<ModTraits>::Int& a, const ModInt<ModTraits>& b) SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<typename ModInt<ModTraits>::Int> && 
			CopyableNoExcept<typename ModInt<ModTraits>::Int>),
		(Addable, typename ModInt<ModTraits>::Int)
	);

	/**
	 * @brief Subtracts an integer value from a modular integer.
	 * @param a Integer value.
	 * @param b Modular integer.
	 * @return Subtraction result.
	 */
	template <ModTraitsType ModTraits>
	inline ModInt<ModTraits> operator-(const typename ModInt<ModTraits>::Int& a, const ModInt<ModTraits>& b) SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<typename ModInt<ModTraits>::Int> && 
			CopyableNoExcept<typename ModInt<ModTraits>::Int>),
		(Addable, typename ModInt<ModTraits>::Int),
		(Subtractable, typename ModInt<ModTraits>::Int)
	);

	/**
	 * @brief Multiplies an integer value with a modular integer.
	 * @param a Integer value.
	 * @param b Modular integer.
	 * @return Multiplication result.
	 */
	template <ModTraitsType ModTraits>
	ModInt<ModTraits> operator*(const typename ModInt<ModTraits>::Int& a, const ModInt<ModTraits>& b) SENC_REQ_NOEXCEPT_COND(
		(ModulableNoExcept<typename ModInt<ModTraits>::Int> && 
			CopyableNoExcept<typename ModInt<ModTraits>::Int>),
		(Multiplicable, typename ModInt<ModTraits>::Int)
	);

	/**
	 * @brief Divides an integer value by a modular integer.
	 * @param a Integer value.
	 * @param b Modular integer.
	 * @return Division result.
	 * @throw ModException If failed to divide (only if `isPrime` is not `true`).
	 */
	template <ModTraitsType ModTraits>
	ModInt<ModTraits> operator/(const typename ModInt<ModTraits>::Int& a, const ModInt<ModTraits>& b) SENC_REQ_NOEXCEPT_COND(
		(ModInt<ModTraits>::IS_PRIME_MOD &&
			ModulableNoExcept<typename ModInt<ModTraits>::Int> &&
			SelfModulableNoExcept<typename ModInt<ModTraits>::Int> &&
			CopyableNoExcept<typename ModInt<ModTraits>::Int>),
		(Multiplicable, typename ModInt<ModTraits>::Int),
		(LowerComparable, typename ModInt<ModTraits>::Int),
		(Andable, typename ModInt<ModTraits>::Int),
		(SelfDevisible, typename ModInt<ModTraits>::Int)
	);

	template <ModTraitsType ModTraits>
	std::ostream& operator<<(std::ostream& os, ModInt<ModTraits> modint)
	requires Outputable<typename ModInt<ModTraits>::Int>;
}

#include "ModInt_impl.hpp"

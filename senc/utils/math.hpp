/*********************************************************************
 * \file   math.hpp
 * \brief  Contains declarations & concepts for math operations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include "concepts.hpp"
#include <cmath>

namespace senc::utils
{
	/**
	 * @concept senc::utils::HasPowMethod
	 * @brief Looks for a typename with a power-computing method.
	 * @tparam Self Examined typename.
	 * @tparam Exponent Exponent type.
	 */
	template <typename Self, typename Exponent>
	concept HasPowMethod = requires(const Self self)
	{
		{ self.pow(std::declval<Exponent>()) } -> std::same_as<Self>;
	};

	/**
	 * @concept senc::utils::SeuqreAndMultiplyCompatible
	 * @brief Looks for a typename that can be used in the square-and-multiplt algorithm.
	 * @tparam Self Examined typename.
	 * @tparam Exponent Exponent type.
	 */
	template <typename Self, typename Exponent>
	concept SquareAndMultiplyCompatible = IntConstructible<Self> &&
		Copyable<Self> && Modulable<Self> && SelfMultiplicable<Self> && 
		LowerComparable<Exponent> && SelfDevisible<Exponent>;

	/**
	 * @concept senc::utils::PowerRaisable
	 * @brief Looks for a typename of which instances can be raised to a power.
	 * @tparam Self Examined typename.
	 * @tparam Exponent Exponent type.
	 */
	template <typename Self, typename Exponent>
	concept PowerRaisable = (std::is_fundamental_v<Self> && std::is_fundamental_v<Exponent>) ||
		HasPowMethod<Self, Exponent> || 
		(SelfMultiplicable<Self> && std::copy_constructible<Self> &&
			IntConstructible<Exponent> && LowerComparable<Exponent> && LeftIncrementable<Exponent>);

	/**
	 * @brief Raises given value to a given power.
	 * @tparam T Value type (must satisfy `PowerRaisable`).
	 * @param val Value to raise to a given power.
	 * @param exp Exponent to use for power raise.
	 * @return Result of raising `val` to the power of `exp`.
	 * If `T` is fundamental, uses `std::pow`.
	 * Otherwise, if `T` satisfies `HasPowMethod`, returns result of `val.pow(exp)`.
	 * Otherwise, computes multiplication in a loop.
	 */
	template <typename T, typename Exponent>
	requires PowerRaisable<T, Exponent>
	inline T pow(const T& val, const Exponent& exp)
	{
		if constexpr (std::is_fundamental_v<T> && std::is_fundamental_v<Exponent>)
		{
			return static_cast<T>(std::pow(
				static_cast<double>(val),
				static_cast<double>(exp)
			));
		}
		else if constexpr (HasPowMethod<T, Exponent>)
		{
			return val.pow(exp);
		}
		else
		{
			T res = val;
			for (Exponent i = 1; i < exp; i++)
				res *= val;
			return res;
		}
	}
}

/*********************************************************************
 * \file   math.hpp
 * \brief  Contains declarations & concepts for math operations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <concepts>
#include <cmath>

namespace senc::utils
{
	/**
	 * @typedef senc::utils::Exponent
	 * @brief Type used as exponent in `pow` function.
	 */
	using Exponent = int;

	/**
	 * @concept senc::utils::SelfMultiplicable
	 * @brief Looks for a typename that can be multiplied by itself.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SelfMultiplicable = requires(Self self)
	{
		{ self *= std::declval<Self>() } -> std::same_as<Self&>;
	};

	/**
	 * @concept senc::utils::HasPowMethod
	 * @brief Looks for a typename with a power-computing method.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasPowMethod = requires(const Self self)
	{
		{ self.pow(std::declval<Exponent>()) } -> std::same_as<Self>;
	};

	/**
	 * @concept senc::utils::PowerRaisable
	 * @brief Looks for a typename of which instances can be raised to a power.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept PowerRaisable = std::is_fundamental_v<Self> ||
		HasPowMethod<Self> || 
		(SelfMultiplicable<Self> && std::copy_constructible<Self>);

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
	template <PowerRaisable T>
	inline T pow(const T& val, Exponent exp)
	{
		if constexpr (std::is_fundamental_v<T>)
		{
			return static_cast<T>(std::pow(
				static_cast<double>(val),
				static_cast<double>(exp)
			));
		}
		else if constexpr (HasPowMethod<T>)
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

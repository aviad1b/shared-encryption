/*********************************************************************
 * \file   swap.hpp
 * \brief  Contains swapping utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include <concepts>

namespace senc::utils
{
	/**
	 * @concept senc::utils::HasSwapMethod
	 * @brief Checks for a typename of which two instances can be swapped using a `swap` method.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasSwapMethod = requires(Self a, Self b)
	{
		{ a.swap(b) };
	};

	/**
	 * @concept senc::utils::StdSwappable
	 * @brief Checks for a typename of which two instances can be swapped using a `std::swap`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept StdSwappable = requires(Self a, Self b)
	{
		{ std::swap(a, b) };
	};

	/**
	 * @brief Swaps two variables.
	 * @tparam T Variable type (must either be `HasSwapMethod` or `StdSwappable`).
	 * @param a First variable to swap.
	 * @param b Second variable to swap.
	 */
	template <typename T>
	requires (HasSwapMethod<T> || StdSwappable<T>)
	void swap(T& a, T& b)
	{
		if constexpr (HasSwapMethod<T>)
			a.swap(b);
		else
			std::swap(a, b);
	}

	/**
	 * @concept senc::utils::Swappable
	 * @brief Checks for a typename of which two instances can be swapped using a `utils::swap`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Swappable = requires(Self a, Self b)
	{
		{ senc::utils::swap(a, b) };
	};
}

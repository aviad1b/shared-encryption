/*********************************************************************
 * \file   Group.hpp
 * \brief  Contains definitions related to algebric groups.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <concepts>

#include "Random.hpp"
#include "math.hpp"

namespace senc::utils
{
	/**
	 * @typedef senc::utils::GroupOrder
	 * @brief Represents an algebric group's order.
	 */
	using GroupOrder = BigInt;

	/**
	 * @concept senc::utils::Group
	 * @brief Looks for a typename that can be used as an algebric group.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Group = std::copyable<Self> && requires(Self self, const Self cself)
	{
		{ Self::order() } -> std::convertible_to<GroupOrder>;
		{ Self::generator() } -> std::convertible_to<Self>;
		{ Self::identity() } -> std::convertible_to<Self>;
		{ cself.is_identity() } -> std::convertible_to<bool>;
		{ cself.inverse() } -> std::same_as<Self>;
		{ std::declval<Self>() * std::declval<Self>() } -> std::same_as<Self>;
		{ self *= std::declval<Self>() } -> std::same_as<Self&>;
		{ std::declval<Self>() / std::declval<Self>() } -> std::same_as<Self>;
		{ self /= std::declval<Self>() } -> std::same_as<Self&>;
	};

	template <typename Self>
	concept PrimeOrderedGroup = Group<Self> && requires
	{
		{ Self::is_prime_ordered() } noexcept -> BoolConvertibleNoExcept;
		{ std::bool_constant<Self::is_prime_ordered()>() }; // must be constexpr-evaluable
	} && Self::is_prime_ordered();

	/**
	 * @concept senc::utils::SamplableGroup
	 * @brief Looks for a typename which satisfies both `Group` and `HasSampleMethod`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SamplableGroup = Group<Self> && HasSampleMethod<Self>;
}

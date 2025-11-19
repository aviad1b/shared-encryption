/*********************************************************************
 * \file   ranges.hpp
 * \brief  Header of range-related utilities.
 * 
 * \author aviad1b
 * \date   November 2025
 *********************************************************************/

#pragma once

#include <optional>
#include <numeric>
#include <ranges>

#include "concepts.hpp"

namespace senc::utils::ranges
{
	/**
	 * @brief Computes product of all elements in range
	 * @param r Range to compute product of.
	 * @return `std::nullopt` is `r` is empty, elements product otherwise.
	 */
	template <std::ranges::input_range R>
	requires Multiplicable<std::ranges::range_value_t<R>>
	std::optional<std::ranges::range_value_t<R>> product(R&& r);
}

#include "ranges_impl.hpp"

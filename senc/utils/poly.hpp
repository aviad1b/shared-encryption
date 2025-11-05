/*********************************************************************
 * \file   poly.hpp
 * \brief  Contains polynomial-related declarations & concepts.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <concepts>

namespace senc::utils
{
	/**
	 * @concept senc::utils::PolyCoeff
	 * @brief Looks for a typename that can be used as a polynom coefficient.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept PolyCoeff = std::movable<Self> && requires
	{
		{ std::declval<Self>() + std::declval<Self>() } -> std::same_as<Self>;
		{ std::declval<Self>() * std::declval<int>() } -> std::same_as<Self>;
	};
}

#include "poly_impl.hpp"

/*********************************************************************
 * \file   concepts.hpp
 * \brief  Contains general concept declarations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <concepts>
#include <iterator>

namespace senc::utils
{
	/**
	 * @concept senc::utils::InputIterator
	 * @brief Looks for a typename that is an input iterator for a given element type.
	 * @tparam Self Examined typename.
	 * @tparam T Element type.
	 */
	template <typename Self, typename T>
	concept InputIterator = std::input_iterator<Self> &&
		std::same_as<std::iter_value_t<Self>, T>;
}

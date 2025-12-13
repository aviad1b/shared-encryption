/*********************************************************************
 * \file   strs.hpp
 * \brief  Contains string-related utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <string>
#include "concepts.hpp"

namespace senc::utils
{
	/**
	 * @brief Looks for a typename with a `to_string` method for string representation.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasToString = requires(const Self self)
	{
		{ self.to_string() } -> std::convertible_to<std::string>;
	};

	/**
	 * @brief Looks for a typename that can be converted to string.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept StringConvertible = HasToString<Self> || Outputable<Self>;

	/**
	 * @brief Looks for a typename with a `from_string` method for string parsing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasFromString = requires(const std::string str)
	{
		{ Self::from_string(str) } -> std::convertible_to<Self>;
	};

	/**
	 * @brief Looks for a typename that can be parsed from string.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept StringParsable = HasFromString<Self> || Inputable<Self>;
}

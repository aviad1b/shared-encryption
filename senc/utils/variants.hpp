/*********************************************************************
 * \file   variants.hpp
 * \brief  Contains variant-related utilities
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <variant>

namespace senc::utils
{
	namespace sfinae
	{
		template <typename... Ts>
		struct variant_or_singular { using type = std::variant<Ts...>; };

		template <typename T>
		struct variant_or_singular<T> { using type = T };
	}

	/**
	 * @typedef senc::utils::VariantOrSingular
	 * @brief Alias of std::variant for multiple types, or for type itself if singular.
	 */
	template <typename... Ts>
	using VariantOrSingular = typename sfinae::variant_or_singular<Ts...>::type;
}

/*********************************************************************
 * \file   Random.hpp
 * \brief  Header of Random<T> class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <concepts>

namespace senc::utils
{
	/**
	 * @concept senc::utils::HasSampleMethod
	 * @brief Looks for a typename of which an instance can be sampled using a `sample` method.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasSampleMethod = requires(const Self self)
	{
		{ self.sample() } -> std::same_as<Self>;
	};

	/**
	 * @concept senc::utils::RandomSamplable
	 * @brief Looks for a typename of which an instance can be sampled.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept RandomSamplable = std::integral<Self> || HasSampleMethod<Self>;
}

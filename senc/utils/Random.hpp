/*********************************************************************
 * \file   Random.hpp
 * \brief  Header of Random<T> class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <functional>
#include <concepts>
#include <random>
#include <chrono>

namespace senc::utils
{
	/**
	 * @class senc::utils::Random
	 * @tparam T Integer type to sample.
	 */
	template <std::integral T>
	class Random
	{
	public:
		Random() = delete; // prevent instancing

		/**
		 * @brief Gets a sample distribution within a given range [min, max].
		 */
		static std::function<T()> get_range_dist(T min, T max);

		/**
		 * @brief Gets a non-negative sample distribution below a given upper bound.
		 */
		static std::function<T()> get_dist_below(T upperBound);
	};
}

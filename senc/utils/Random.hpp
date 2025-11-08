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
	template <std::integral T>
	class Distribution
	{
		friend class Random<T>;

	public:
		using Self = Distribution<T>;

		Distribution(const Self&) = default;

		Self& operator=(const Self&) = default;

		T operator()() const noexcept;

	private:
		std::uniform_int_distribution<T> _dist;
		std::mt19937& _engine;

		Distribution(T min, T max, std::mt19937& engine);
	};

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
		static Distribution<T> get_range_dist(T min, T max) noexcept;

		/**
		 * @brief Gets a non-negative sample distribution below a given upper bound.
		 */
		static Distribution<T> get_dist_below(T upperBound) noexcept;

		/**
		 * @brief Samples a number within a given range [min, max].
		 */
		static T sample_from_range(T min, T max) noexcept;

		/**
		 * @brief Samples a non-genative number below a given upper bound.
		 */
		static T sample_below(T upperBound) noexcept;

	private:
		static thread_local std::mt19937& engine() noexcept
		{
			static thread_local std::mt19937 eng(
				std::chrono::high_resolution_clock::now().time_since_epoch().count()
			);
			return eng;
		}
	};
}

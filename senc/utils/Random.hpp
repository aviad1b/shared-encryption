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
	 * @class senc::utils::Distribution
	 * @brief Represents a uniform distribution of integers.
	 * @tparam T Integer type.
	 */
	template <std::integral T>
	class Distribution
	{
		friend class Random<T>;

	public:
		using Self = Distribution<T>;

		/**
		 * @brief Copy constructor of distribution.
		 */
		Distribution(const Self&) = default;

		/**
		 * @brief Copy assignment operator distribution.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Samples a random integer from distribution.
		 */
		T operator()() const noexcept;

	private:
		std::uniform_int_distribution<T> _dist;
		std::mt19937& _engine;

		/**
		 * @brief Constructs a uniform integer distribution (range [min, max]).
		 * @param min Minimum value in range.
		 * @param max Maximum value in range.
		 * @param engine Engine used for random generations (by ref).
		 */
		Distribution(T min, T max, std::mt19937& engine);
	};

	/**
	 * @concept senc::utils::HasSampleMethod
	 * @brief Looks for a typename of which an instance can be sampled using a `sample` method.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasSampleMethod = requires
	{
		{ Self::sample() } -> std::same_as<Self>;
	};

	/**
	 * @concept senc::utils::RandomSamplable.
	 * @brief Looks for a typename that can be sampled through the `Random` class in any way.
	 * @tparam Self Examiend typename.
	 */
	template <typename Self>
	concept RandomSamplable = std::integral<Self> || HasSampleMethod<Self>;

	/**
	 * @class senc::utils::Random
	 * @brief Class containing static functions for random sampling.
	 * @tparam T Type being sampled.
	 */
	template <RandomSamplable T>
	class Random
	{
	public:
		Random() = delete; // prevent instancing

		/**
		 * @brief Gets a sample distribution within a given range [min, max].
		 * @note Requires `T` to be an integer type.
		 */
		static Distribution<T> get_range_dist(T min, T max) noexcept
		requires std::integral<T>;

		/**
		 * @brief Gets a non-negative sample distribution below a given upper bound.
		 * @note Requires `T` to be an integer type.
		 */
		static Distribution<T> get_dist_below(T upperBound) noexcept
		requires std::integral<T>;

		/**
		 * @brief Samples a number within a given range [min, max].
		 * @note Requires `T` to be an integer type.
		 */
		static T sample_from_range(T min, T max) noexcept
		requires std::integral<T>;

		/**
		 * @brief Samples a non-genative number below a given upper bound.
		 * @note Requires `T` to be an integer type.
		 */
		static T sample_below(T upperBound) noexcept
		requires std::integral<T>;

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

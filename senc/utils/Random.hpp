/*********************************************************************
 * \file   Random.hpp
 * \brief  Header of Random<T> class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <functional>
#include <concepts>
#include <random>
#include <chrono>

namespace senc::utils
{
	/**
	 * @concept senc::utils::UnderlyingDistType
	 * @brief Looks for typename that can be used as underlying distribution type.
	 * @tparam Self Examined typename.
	 * @tparam T Type being sampled.
	 */
	template <typename Self, typename T>
	concept UnderlyingDistType = std::copyable<Self> &&
		std::constructible_from<Self, const T&, const T&> &&
		requires(const Self self, DistEngine<T>& engine)
		{
			{ self(engine) } -> std::same_as<T>;
		};

	/**
	 * @concept senc::utils::UnderlyingDistType
	 * @brief Looks for typename that can be used as underlying distribution type,
	 *		  with `noexcept` sampling.
	 * @tparam Self Examined typename.
	 * @tparam T Type being sampled.
	 */
	template <typename Self, typename T>
	concept UnderlyingDistTypeNoExcept = UnderlyingDistType<Self, T> &&
		requires(const Self self, DistEngine<T>& engine)
		{
			{ self(engine) } noexcept -> std::same_as<T>;
		};

	/**
	 * @brief Underlying class used for sampling `CryptoPP::Integer`.
	 */
	class CryptoUnderlyingDist
	{
	public:
		using Self = CryptoUnderlyingDist;

		CryptoUnderlyingDist(const CryptoPP::Integer& min, const CryptoPP::Integer& max);

		CryptoUnderlyingDist(const Self&) = default;

		Self& operator=(const Self&) = default;

		CryptoUnderlyingDist(Self&&) = default;

		Self& operator=(Self&&) = default;

		CryptoPP::Integer operator()(CryptoPP::RandomNumberGenerator& engine) const;

	private:
		CryptoPP::Integer _min, _max;
	};

	namespace sfinae
	{
		template <typename T>
		struct dist_engine { };

		template <std::integral T>
		struct dist_engine<T> { using type = std::mt19937; };

		template <>
		struct dist_engine<CryptoPP::Integer> { using type = CryptoPP::RandomNumberGenerator; };

		template <typename T>
		struct underlying_dist { };

		template <std::integral T>
		struct underlying_dist<T> { using type = std::uniform_int_distribution<T>; };

		template <>
		struct underlying_dist<CryptoPP::Integer> { using type = CryptoUnderlyingDist; };
	}

	/**
	 * @typedef senc::utils::DistEngine
	 * @brief Engine used for sampling a distribution value.
	 * @tparam T Distribution value type.
	 */
	template <typename T>
	using DistEngine = typename sfinae::dist_engine<T>::type;

	/**
	 * @brief Underlying distribution type used by Distribution class.
	 * @tparam T Distribution value type.
	 */
	template <typename T>
	using UnderlyingDist = typename sfinae::underlying_dist<T>::type;

	/**
	 * @concept senc::utils::DistEngineType
	 * @brief Looks for typename which can be used as engine for sampling in Distribution class.
	 * @tparam Self Examined typename.
	 * @tparam T Type being sampled.
	 */
	template <typename Self, typename T>
	concept DistEngineType = true; // no constraints

	/**
	 * @concept senc::utils::DistVal
	 * @brief Looks for a typename that can be sampled from `Distribution`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept DistVal = DistEngineType<DistEngine<Self>, Self> && 
		UnderlyingDistType<UnderlyingDist<Self>, Self>;

	/**
	 * @class senc::utils::Distribution
	 * @brief Represents a uniform distribution of values.
	 * @tparam T Value type.
	 */
	template <DistVal T>
	class Distribution
	{
	public:
		using Self = Distribution<T>;

		/**
		 * @brief Constructs a uniform distribution (range [min, max]).
		 * @param min Minimum value in range.
		 * @param max Maximum value in range.
		 * @param engine Engine used for random generations (by ref).
		 */
		Distribution(T min, T max, DistEngine<T>& engine);

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
		T operator()() const noexcept(UnderlyingDistTypeNoExcept<UnderlyingDist<T>>);

	private:
		UnderlyingDist<T> _dist;
		DistEngine<T>& _engine;
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
	concept RandomSamplable = DistVal<Self> || HasSampleMethod<Self>;

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
		requires DistVal<T>;

		/**
		 * @brief Samples a number within a given range [min, max].
		 * @note Requires `T` to be an integer type.
		 */
		static T sample_from_range(T min, T max) noexcept
		requires DistVal<T>;

		/**
		 * @brief Samples a non-genative number below a given upper bound.
		 * @note Requires `T` to be an integer type.
		 */
		static T sample_below(T upperBound) noexcept
		requires DistVal<T>;

	private:
		static thread_local std::mt19937& engine() noexcept
		{
			static thread_local std::mt19937 eng(
				std::chrono::high_resolution_clock::now().time_since_epoch().count()
			);
			return eng;
		}

		static thread_local CryptoPP::RandomNumberGenerator& engine_crypto()
		{
			static thread_local CryptoPP::AutoSeededRandomPool rng;
			return rng;
		}
	};
}

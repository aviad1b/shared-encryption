/*********************************************************************
 * \file   Random.hpp
 * \brief  Header of randomization utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

// this include is needed because CryptoPP uses WinAPI
#include "../utils/winapi_patch.hpp"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <functional>
#include <concepts>
#include <random>
#include <chrono>

#include "math.hpp"

namespace senc::utils
{
	namespace sfinae
	{
		// used for converting a premitive into a "canon" randomizable type
		// (based on its size)
		template <typename T>
		struct fundamental_underlying_dist_value { };

		template <typename T>
		requires (
			sizeof(T) <= sizeof(short) &&
			std::signed_integral<T>
		)
		struct fundamental_underlying_dist_value<T> { using type = short; };

		template <typename T>
		requires (
			sizeof(T) <= sizeof(unsigned short) &&
			!std::signed_integral<T>
		)
		struct fundamental_underlying_dist_value<T> { using type = unsigned short; };

		template <typename T>
		requires (
			sizeof(short) < sizeof(T) &&
			sizeof(T) <= sizeof(int) &&
			std::signed_integral<T>
		)
		struct fundamental_underlying_dist_value<T> { using type = int; };

		template <typename T>
		requires (
			sizeof(unsigned short) < sizeof(T) &&
			sizeof(T) <= sizeof(unsigned int) &&
			!std::signed_integral<T>
		)
		struct fundamental_underlying_dist_value<T> { using type = unsigned int; };

		template <typename T>
		requires (
			sizeof(int) < sizeof(T) &&
			sizeof(T) <= sizeof(long) &&
			std::signed_integral<T>
		)
		struct fundamental_underlying_dist_value<T> { using type = long; };

		template <typename T>
		requires (
			sizeof(unsigned int) < sizeof(T) &&
			sizeof(T) <= sizeof(unsigned long) &&
			!std::signed_integral<T>
		)
		struct fundamental_underlying_dist_value<T> { using type = unsigned long; };

		template <typename T>
		requires (
			sizeof(long) < sizeof(T) &&
			sizeof(T) <= sizeof(long long) &&
			std::signed_integral<T>
		)
		struct fundamental_underlying_dist_value<T> { using type = long long; };

		template <typename T>
		requires (
			sizeof(unsigned long) < sizeof(T) &&
			sizeof(T) <= sizeof(unsigned long long) &&
			!std::signed_integral<T>
		)
		struct fundamental_underlying_dist_value<T> { using type = unsigned long long; };
	}

	/**
	 * @concept senc::utils::PrimitiveSamplable
	 * @brief Looks for a primitive (fundamental / enum) typename that can be sampled.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept PrimitiveSamplable = requires
	{
		typename sfinae::fundamental_underlying_dist_value<Self>::type;
	};

	/**
	 * @brief Underlying class used for sampling fundamental/enum types.
	 */
	template <PrimitiveSamplable T>
	class PrimitiveUnderlyingDist
	{
	public:
		using Self = PrimitiveUnderlyingDist<T>;
		using BaseVal = typename sfinae::fundamental_underlying_dist_value<T>::type;

		PrimitiveUnderlyingDist(T min, T max)
			: _base(static_cast<BaseVal>(min), static_cast<BaseVal>(max)) { }

		PrimitiveUnderlyingDist(const Self&) = default;

		Self& operator=(const Self&) = default;

		PrimitiveUnderlyingDist(Self&&) = default;

		Self& operator=(Self&&) = default;

		T operator()(auto& engine) const
		{
			return static_cast<T>(_base()(engine));
		}

	private:
		std::uniform_int_distribution<BaseVal> _base;
	};

	/**
	 * @brief Underlying class used for sampling `BigInt`.
	 */
	class BigIntUnderlyingDist
	{
	public:
		using Self = BigIntUnderlyingDist;

		BigIntUnderlyingDist(const BigInt& min, const BigInt& max)
			: _min(min), _max(max) { }

		BigIntUnderlyingDist(const Self&) = default;

		Self& operator=(const Self&) = default;

		BigIntUnderlyingDist(Self&&) = default;

		Self& operator=(Self&&) = default;

		BigInt operator()(CryptoPP::RandomNumberGenerator& engine) const
		{
			BigInt res;
			res.Randomize(
				engine, this->_min, this->_max,
				BigInt::RandomNumberType::ANY
			);
			return res;
		}

	private:
		BigInt _min, _max;
	};

	namespace sfinae
	{
		template <typename T>
		struct dist_engine { };

		template <PrimitiveSamplable T>
		struct dist_engine<T> { using type = std::mt19937; };

		template <>
		struct dist_engine<BigInt> { using type = CryptoPP::RandomNumberGenerator; };

		template <typename T>
		struct underlying_dist { };

		template <PrimitiveSamplable T>
		struct underlying_dist<T> { using type = PrimitiveUnderlyingDist<T>; };

		template <>
		struct underlying_dist<BigInt> { using type = BigIntUnderlyingDist; };
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
	 * @concept senc::utils::UnderlyingDistType
	 * @brief Looks for typename that can be used as underlying distribution type.
	 * @tparam Self Examined typename.
	 * @tparam T Type being sampled.
	 */
	template <typename Self, typename T>
	concept UnderlyingDistType = std::copyable<Self> &&
		std::constructible_from<Self, const T&, const T&> &&
		requires(Self& self, DistEngine<T>& engine)
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
		requires(Self& self, DistEngine<T>& engine)
		{
			{ self(engine) } noexcept -> std::same_as<T>;
		};

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

	// safety static asserts: types that should satisfy `DistVal`
	static_assert(DistVal<int>);
	static_assert(DistVal<BigInt>);

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
		Distribution(const T& min, const T& max, DistEngine<T>& engine);

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
		T operator()() noexcept(UnderlyingDistTypeNoExcept<UnderlyingDist<T>, T>);

		/**
		 * @brief Samples a random value from distribution, such that doesn't match a given predicate.
		 * @param invalidPred A predicate that the sampled value shouldn't hold.
		 * @return Sampled value.
		 */
		T operator()(Callable<bool, const T> auto&& invalidPred)
			noexcept(UnderlyingDistTypeNoExcept<UnderlyingDist<T>, T> &&
					 CallableNoExcept<std::remove_cvref_t<decltype(invalidPred)>, bool, const T>);

		/**
		 * @brief Samples a unique random value from distribution.
		 * @param container Object containing existing values (to check uniqueness).
		 * @return Sampled value.
		 */
		T operator()(const HasContainsMethod<T> auto& container)
			noexcept(UnderlyingDistTypeNoExcept<UnderlyingDist<T>, T> &&
					 HasContainsMethodNoExcept<std::remove_cvref_t<decltype(container)>, T>);

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
	 * @concept senc::utils::HasSampleMethod
	 * @brief Looks for a typename of which an instance can be sampled using a `sample` method,
	 *		  without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasSampleMethodNoExcept = requires
	{
		{ Self::sample() } noexcept -> std::same_as<Self>;
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
		 * @brief Gets a sample distribution.
		 * @note Requires `T` to be an integer type.
		 */
		static Distribution<T> get_dist() noexcept
		requires std::integral<T>;

		/**
		 * @brief Samples random instance.
		 */
		static T sample() noexcept(HasSampleMethodNoExcept<T>)
		requires HasSampleMethod<T>;

		/**
		 * @brief Samples random instance.
		 */
		static T sample() noexcept
		requires std::integral<T>;

		/**
		 * @brief Gets a sample distribution within a given range [min, max].
		 * @note Requires `T` to be an integer type.
		 */
		static Distribution<T> get_range_dist(const T& min, const T& max) noexcept
		requires DistVal<T>;

		/**
		 * @brief Gets a non-negative sample distribution below a given upper bound.
		 * @note Requires `T` to be an integer type.
		 */
		static Distribution<T> get_dist_below(const T& upperBound) noexcept
		requires DistVal<T>;

		/**
		 * @brief Samples a number within a given range [min, max].
		 * @note Requires `T` to be an integer type.
		 */
		static T sample_from_range(const T& min, const T& max) noexcept
		requires DistVal<T>;

		/**
		 * @brief Samples a non-genative number below a given upper bound.
		 * @note Requires `T` to be an integer type.
		 */
		static T sample_below(const T& upperBound) noexcept
		requires DistVal<T>;

	private:
		static std::mt19937& engine() noexcept
		{
			static thread_local std::mt19937 eng(
				std::chrono::high_resolution_clock::now().time_since_epoch().count()
			);
			return eng;
		}

		static CryptoPP::RandomNumberGenerator& engine_crypto()
		{
			static thread_local CryptoPP::AutoSeededRandomPool rng;
			return rng;
		}
	};
}

#include "Random_impl.hpp"

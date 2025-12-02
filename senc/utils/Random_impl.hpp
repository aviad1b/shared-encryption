/*********************************************************************
 * \file   Random_impl.hpp
 * \brief  Implementation of Random<T> class.
 * 
 * \author test
 * \date   November 2025
 *********************************************************************/

#include "Random.hpp"

#include <limits>

namespace senc::utils
{
	template <DistVal T>
	inline Distribution<T>::Distribution(const T& min, const T& max, DistEngine<T>& engine)
		: _dist(min, max), _engine(engine) { }

	template <DistVal T>
	inline T Distribution<T>::operator()() noexcept(UnderlyingDistTypeNoExcept<UnderlyingDist<T>, T>)
	{
		return this->_dist(this->_engine);
	}

	template <RandomSamplable T>
	inline Distribution<T> Random<T>::get_dist() noexcept
	requires std::integral<T>
	{
		return get_range_dist(
			std::numeric_limits<T>::min(),
			std::numeric_limits<T>::max()
		);
	}

	template <RandomSamplable T>
	inline T Random<T>::sample() noexcept(HasSampleMethodNoExcept<T>)
	requires HasSampleMethod<T>
	{
		return T::sample();
	}

	template <RandomSamplable T>
	inline T Random<T>::sample() noexcept
	requires std::integral<T>
	{
		return sample_from_range(
			std::numeric_limits<T>::min(),
			std::numeric_limits<T>::max()
		);
	}

	template <RandomSamplable T>
	inline Distribution<T> Random<T>::get_range_dist(const T& min, const T& max) noexcept
	requires DistVal<T>
	{
		if constexpr(std::same_as<T, BigInt>)
			return Distribution<T>(min, max, engine_crypto());
		else
			return Distribution<T>(min, max, engine());
	}

	template <RandomSamplable T>
	inline Distribution<T> Random<T>::get_dist_below(const T& upperBound) noexcept
	requires DistVal<T>
	{
		return get_range_dist(0, upperBound - 1);
	}

	template <RandomSamplable T>
	inline T Random<T>::sample_from_range(const T& min, const T& max) noexcept
	requires DistVal<T>
	{
		return get_range_dist(min, max)();
	}
	
	template <RandomSamplable T>
	inline T Random<T>::sample_below(const T& upperBound) noexcept
	requires DistVal<T>
	{
		return get_dist_below(upperBound)();
	}
}

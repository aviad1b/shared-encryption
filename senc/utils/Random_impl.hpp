/*********************************************************************
 * \file   Random_impl.hpp
 * \brief  Implementation of Random<T> class.
 * 
 * \author test
 * \date   November 2025
 *********************************************************************/

#include "Random.hpp"

namespace senc::utils
{
	template <DistVal T>
	inline Distribution<T>::Distribution(T min, T max, DistEngine<T>& engine)
		: _dist(min, max), _engine(engine) { }

	template <DistVal T>
	inline T Distribution<T>::operator()() const noexcept(UnderlyingDistTypeNoExcept<UnderlyingDist<T>, T>)
	{
		return this->_dist(this->_engine);
	}

	template <RandomSamplable T>
	inline Distribution<T> Random<T>::get_range_dist(T min, T max) noexcept
	requires std::integral<T>
	{
		if constexpr(std::same_as<T, CryptoPP::Integer>)
			return Distribution<T>(min, max, engine_crypto());
		else
			return Distribution<T>(min, max, engine());
	}

	template <RandomSamplable T>
	inline Distribution<T> Random<T>::get_dist_below(T upperBound) noexcept
	requires DistVal<T>
	{
		return get_range_dist(0, upperBound - 1);
	}

	template <RandomSamplable T>
	inline T Random<T>::sample_from_range(T min, T max) noexcept
	requires DistVal<T>
	{
		return get_range_dist(min, max)();
	}
	
	template <RandomSamplable T>
	inline T Random<T>::sample_below(T upperBound) noexcept
	requires DistVal<T>
	{
		return get_dist_below(upperBound)();
	}
}

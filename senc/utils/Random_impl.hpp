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
	template <std::integral T>
	inline std::function<T()> Random<T>::get_range_dist(T min, T max)
	{
		return [dist = std::uniform_int_distribution<T>(min, max)]() mutable
		{
            static thread_local std::mt19937 engine(
                std::chrono::high_resolution_clock::now().time_since_epoch().count()
            );
            return dist(engine);
        };
	}
	
	template <std::integral T>
	inline std::function<T()> Random<T>::get_dist_below(T upperBound)
	{
		return get_range_dist(0, upperBound - 1);
	}
}

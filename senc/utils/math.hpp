/*********************************************************************
 * \file   math.hpp
 * \brief  Contains declarations & concepts for math operations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <concepts>
#include <cmath>

namespace senc::utils
{
	template <typename T>
	requires std::is_fundamental_v<T>
	inline T pow(T val, int exp)
	{
		return static_cast<T>(std::pow(
			static_cast<double>(val),
			static_cast<double>(exp)
		));
	}
}

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
	/**
	 * @concept senc::utils::SelfMultiplicable
	 * @brief Looks for a typename that can be multiplied by itself.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SelfMultiplicable = requires(Self self)
	{
		{ self *= std::declval<Self>() } -> std::same_as<Self&>;
	};

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

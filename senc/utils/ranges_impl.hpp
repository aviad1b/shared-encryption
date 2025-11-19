/*********************************************************************
 * \file   ranges_impl.hpp
 * \brief  Template implementation of range-related utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "ranges.hpp"

namespace senc::utils::ranges
{
	template <std::ranges::input_range R>
	requires Multiplicable<std::ranges::range_value_t<R>>
	inline std::optional<std::ranges::range_value_t<R>> product(R&& r)
	{
		auto it = std::ranges::begin(r);
		auto end = std::ranges::end(r);

		if (it == end)
			return std::nullopt;

		auto res = *it;
		++it;

		return std::accumulate(
			it, end, res,
			std::multiplies<std::ranges::range_value_t<R>>{}
		);
	};
}

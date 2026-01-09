/*********************************************************************
 * \file   uuid_impl.hpp
 * \brief  Template implementation of UUID utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "uuid.hpp"

namespace senc::utils
{
	inline UUID::Self UUID::generate_not_pred(Callable<bool, const Self&> auto&& existsPred)
	{
		Self res = generate();
		while (existsPred(res))
			res = generate();
		return res;
	}

	inline UUID::Self UUID::generate_not_in(const HasContainsMethod<Self> auto& container)
	{
		return generate_not_pred([&container](const Self& self) { return container.contains(self); });
	}
}

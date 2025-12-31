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
	inline UUID::Self UUID::generate(Callable<bool, const Self&> auto&& existsPred)
		noexcept(CallableNoExcept<std::remove_cvref_t<decltype(existsPred)>, bool, const Self&>)
	{
		Self res = generate();
		while (existsPred(res))
			res = generate();
		return res;
	}

	inline UUID::Self UUID::generate(const HasContainsMethod<Self> auto& container)
		noexcept(HasContainsMethodNoExcept<std::remove_cvref_t<decltype(container)>, Self>)
	{
		return generate([&container](const Self& self) { return container.contains(self); });
	}
}

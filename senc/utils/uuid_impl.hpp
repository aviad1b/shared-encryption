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
	UUID::Self UUID::generate(Callable<bool, const Self&> auto&& existsPred)
	{
		Self res = generate();
		while (existsPred(res))
			res = generate();
		return res;
	}
}

/*********************************************************************
 * \file   shamir_impl.hpp
 * \brief  Implementation of Shamir class (secret sharing utilities).
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "shamir.hpp"

namespace senc::utils
{
	template <typename S, ShamirShardID SID>
	inline Shamir<S, SID>::Poly Shamir<S, SID>::sample_poly(
		const S& secret, Threshold threshold, std::function<S()> secretSampler)
	{
		return Poly::sample(threshold, [=]() -> PackedSecret { return secretSampler(); });
	}
}

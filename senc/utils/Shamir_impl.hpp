/*********************************************************************
 * \file   shamir_impl.hpp
 * \brief  Implementation of Shamir class (secret sharing utilities).
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "shamir.hpp"

#include <unordered_set>

namespace senc::utils
{
	template <typename S, ShamirShardID SID>
	inline Shamir<S, SID>::Poly Shamir<S, SID>::sample_poly(
		const S& secret, Threshold threshold, std::function<S()> secretSampler)
	{
		return Poly::sample(threshold, [=]() -> PackedSecret { return secretSampler(); });
	}

	template <typename S, ShamirShardID SID>
	inline Shamir<S, SID>::Shard Shamir<S, SID>::make_shard(const Poly& poly, SID shardID)
	{
		if (!shardID)
			throw ShamirException("Invalid shard ID provided: Should be non-zero");
		return { shardID, poly(shardID) };
	}

	template <typename S, ShamirShardID SID>
	inline std::vector<Shamir<S, SID>::Shard> 
		Shamir<S, SID>::make_shards(const Poly& poly, const std::vector<SID>& shardsIDs)
	{
		std::unordered_set<SID> usedIDs;
		std::vector<Shard> res;
		for (const SID& shardID : shardsIDs)
		{
			if (usedIDs.contains(shardID))
				throw ShamirException(std::format("Same ID provided twice, IDs should be unique"), shardID);
			res.push_back(make_shard(poly, shardID));
			usedIDs.insert(shardID);
		}
		return res;
	}
}

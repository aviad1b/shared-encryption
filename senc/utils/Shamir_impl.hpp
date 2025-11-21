/*********************************************************************
 * \file   shamir_impl.hpp
 * \brief  Implementation of Shamir class (secret sharing utilities).
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Shamir.hpp"

#include <unordered_set>

#include "ranges.hpp"

namespace senc::utils
{
	template <typename S, ShamirShardID SID>
	requires ShamirSecret<S, SID>
	inline typename Shamir<S, SID>::Poly Shamir<S, SID>::sample_poly(
		const S& secret, Threshold threshold, std::function<S()> secretSampler)
	{
		return Poly::sample(
			threshold,
			[=]() -> PackedSecret { return secretSampler(); },
			PackedSecret(secret)
		);
	}

	template <typename S, ShamirShardID SID>
	requires ShamirSecret<S, SID>
	inline typename Shamir<S, SID>::Shard Shamir<S, SID>::make_shard(const Poly& poly, SID shardID)
	{
		if (!shardID)
			throw ShamirException("Invalid shard ID provided: Should be non-zero");
		return { shardID, poly(shardID) };
	}

	template <typename S, ShamirShardID SID>
	requires ShamirSecret<S, SID>
	template <std::ranges::input_range R>
	requires std::convertible_to<std::ranges::range_value_t<R>, SID>
	inline std::vector<typename Shamir<S, SID>::Shard> 
		Shamir<S, SID>::make_shards(const Poly& poly, R&& shardsIDs)
	{
		std::unordered_set<SID> usedIDs;
		std::vector<Shard> res;
		for (const SID& shardID : shardsIDs)
		{
			if (usedIDs.contains(shardID))
				throw ShamirException("Same ID provided twice, IDs should be unique");
			res.push_back(make_shard(poly, shardID));
			usedIDs.insert(shardID);
		}
		return res;
	}

	template <typename S, ShamirShardID SID>
	requires ShamirSecret<S, SID>
	inline S Shamir<S, SID>::restore_secret(const std::vector<Shard>& shards, Threshold threshold)
	{
		std::vector<SID> shardsIDs = utils::to_vector<SID>(
			shards |
			std::views::transform([](const Shard& shard) -> const SID& { return shard.first; })
		);
		if (shards.size() <= static_cast<std::size_t>(threshold))
			throw ShamirException("Not enough shards provided to restore secret");
		PackedSecret res = utils::sum(
			shards |
			std::views::transform([](const Shard& shard) -> const PackedSecret& { return shard.second; }) |
			views::enumerate |
			std::views::transform([&shardsIDs](auto p) -> PackedSecret
			{
				const auto& [i, yi] = p; // shard index, shard value
				return yi * get_lagrange_coeff(i, shardsIDs);
			})
		).value();

		// if original was integral, unpack from fraction
		if constexpr (std::integral<S>)
		{
			if (1 != res.denominator())
				throw ShamirException("Failed to restore integral type");
			return res.numerator();
		}
		else return res; // if not integral, S is same as PackedSecret; return
	}

	template <typename S, ShamirShardID SID>
	requires ShamirSecret<S, SID>
	inline typename Shamir<S, SID>::PackedSecret Shamir<S, SID>::get_lagrange_coeff(
		std::size_t i, const std::vector<SID> shardsIDs)
	{
		std::unordered_set<SID> shardsIDsSet;
		for (const SID& shardID : shardsIDs)
		{
			if (!shardID)
				throw ShamirException("Invalid ID provided: Should be non-zero");
			shardsIDsSet.insert(shardID);
		}
		if (shardsIDs.size() != shardsIDsSet.size())
			throw ShamirException("Invalid IDs provided: Not unique");
		const auto& xi = shardsIDs[i];
		return utils::product(
			shardsIDs |
			views::enumerate | // p = pair{j, xj}
			std::views::filter([i](const auto& p) { return p.first != i; }) | // filter where index isn't i
			std::views::transform([](const auto& p) { return -p.second / xi - p.second; })
		).value();
	}
}

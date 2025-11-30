/*********************************************************************
 * \file   shamir_impl.hpp
 * \brief  Implementation of Shamir class (secret sharing utilities).
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Shamir.hpp"

#include "ranges.hpp"
#include "hash.hpp"

namespace senc::utils
{
	template <typename S, ShamirShardID SID>
	requires ShamirSecret<S, SID>
	inline typename ShamirUtils<S, SID>::Shard ShamirUtils<S, SID>::make_shard(const Poly& poly, SID shardID)
	{
		if (!shardID)
			throw ShamirException("Invalid shard ID provided: Should be non-zero");
		return { shardID, poly(shardID) };
	}

	template <typename S, ShamirShardID SID>
	requires ShamirSecret<S, SID>
	template <std::ranges::input_range R>
	requires std::convertible_to<std::ranges::range_value_t<R>, SID>
	inline std::vector<typename ShamirUtils<S, SID>::Shard> 
		ShamirUtils<S, SID>::make_shards(const Poly& poly, R&& shardsIDs)
	{
		HashSet<SID> usedIDs;
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
	inline typename ShamirUtils<S, SID>::PackedSecret ShamirUtils<S, SID>::get_lagrange_coeff(
		std::size_t i, const std::vector<SID>& shardsIDs)
	{
		// TODO: This check may lower performance, is it REALLY necessary?
		HashSet<SID> shardsIDsSet;
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
			std::views::filter([i](auto p) { return p.first != i; }) | // filter where index isn't i
			std::views::transform([xi](auto p)
			{
				return static_cast<PackedSecret>(p.second) / (
					static_cast<PackedSecret>(p.second) - static_cast<PackedSecret>(xi)
				);
			})
		);
	}

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
				return yi * Utils::get_lagrange_coeff(i, shardsIDs);
			})
		);

		// if original was integral, unpack from fraction
		if constexpr (std::integral<S>)
		{
			if (1 != res.denominator())
				throw ShamirException("Failed to restore integral type");
			return res.numerator();
		}
		else return res; // if not integral, S is same as PackedSecret; return
	}

	template <Group G, enc::Symmetric1L SE, ConstCallable<enc::Key<SE>, G, G> KDF, ShamirShardID SID>
	inline ShamirHybridElGamal<G, SE, KDF, SID>::Poly
		ShamirHybridElGamal<G, SE, KDF, SID>::sample_poly(const BigInt& privKey, Threshold threshold)
	{
		return Shamir<S, SID>::sample_poly(
			privKey, threshold, S::sample
		);
	}

	template <Group G, enc::Symmetric1L SE, ConstCallable<enc::Key<SE>, G, G> KDF, ShamirShardID SID>
	template <int layer>
	requires (1 == layer || 2 == layer)
	inline ShamirHybridElGamal<G, SE, KDF, SID>::Part
		ShamirHybridElGamal<G, SE, KDF, SID>::decrypt_get_2l(
			const Ciphertext& ciphertext,
			const Shard& privKeyShard,
			const std::vector<SID>& privKeyShardsIDs)
	{
		const G& c = std::get<layer - 1>(ciphertext);
		const auto& [xi, yi] = privKeyShard;

		// TODO: Code duplication?
		HashSet<SID> privKeyShardsIDsSet;
		std::optional<std::size_t> i;
		for (const auto& [idx, shardID] : privKeyShardsIDs | views::enumerate)
		{
			if (!shardID)
				throw ShamirException("Invalid ID provided: Should be non-zero");
			privKeyShardsIDsSet.insert(shardID);
			if (shardID == xi)
				i = idx;
		}
		if (privKeyShardsIDs.size() != privKeyShardsIDsSet.size())
			throw ShamirException("Invalid IDs provided: Not unique");
		if (!i.has_value())
			throw ShamirException("Shard with ID no present");

		return utils::pow(
			c,
			yi * Utils::get_lagrange_coeff(i.value(), privKeyShardsIDs)
		);
	}

	template <Group G, enc::Symmetric1L SE, ConstCallable<enc::Key<SE>, G, G> KDF, ShamirShardID SID>
	inline ShamirHybridElGamal<G, SE, KDF, SID>::Plaintext
		ShamirHybridElGamal<G, SE, KDF, SID>::decrypt_join_2l(
			const Ciphertext& ciphertext,
			const std::vector<Part>& parts1,
			const std::vector<Part>& parts2)
	{
		const auto& c3 = std::get<2>(ciphertext);
		auto z1 = utils::product(parts1);
		auto z2 = utils::product(parts2);

		auto k = _kdf(z1, z2);

		return _symmetricSchema.decrypt(c3, k);
	}

	template <Group G, enc::Symmetric1L SE, ConstCallable<enc::Key<SE>, G, G> KDF, ShamirShardID SID>
	inline SE ShamirHybridElGamal<G, SE, KDF, SID>::_symmetricSchema;

	template <Group G, enc::Symmetric1L SE, ConstCallable<enc::Key<SE>, G, G> KDF, ShamirShardID SID>
	inline KDF ShamirHybridElGamal<G, SE, KDF, SID>::_kdf;
}

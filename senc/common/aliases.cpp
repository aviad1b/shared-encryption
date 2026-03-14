/*********************************************************************
 * \file   aliases.cpp
 * \brief  Implementation of utilities for type aliases.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "aliases.hpp"

namespace senc
{
	utils::BytesView::iterator read_pub_key(PubKey& out,
											utils::BytesView::iterator it,
											utils::BytesView::iterator end)
	{
		static thread_local utils::Buffer pubKeyBuff(PubKey::ENCODED_SIZE);
		it = utils::read_bytes(pubKeyBuff, it, end);
		out = utils::from_bytes<PubKey>(pubKeyBuff);
		return it;
	}

	PubKey pub_key_from_bytes(utils::BytesView bytes)
	{
		PubKey res{};
		read_pub_key(res, bytes.begin(), bytes.end());
		return res;
	}

	void write_pub_key(utils::Buffer& out, const PubKey& pubKey)
	{
		utils::write_bytes(out, utils::to_bytes(pubKey));
	}

	utils::Buffer pub_key_to_bytes(const PubKey& pubKey)
	{
		utils::Buffer res{};
		write_pub_key(res, pubKey);
		return res;
	}

	utils::BytesView::iterator read_priv_key_shard(PrivKeyShard& out,
												   utils::BytesView::iterator it,
												   utils::BytesView::iterator end)
	{
		static thread_local utils::Buffer shardIdBuff(SHARD_ID_MAX_SIZE);
		static thread_local utils::Buffer shardValBuff(SHARD_VALUE_MAX_SIZE);
		static thread_local utils::BigInt shardValUnderlying{};
		it = utils::read_bytes(shardIdBuff, it, end);
		it = utils::read_bytes(shardValBuff, it, end);
		out.first.Decode(shardIdBuff.data(), shardIdBuff.size());
		shardValUnderlying.Decode(shardValBuff.data(), shardValBuff.size());
		out.second = std::move(shardValUnderlying);
		return it;
	}

	PrivKeyShard priv_key_shard_from_bytes(utils::BytesView bytes)
	{
		PrivKeyShard res{};
		read_priv_key_shard(res, bytes.begin(), bytes.end());
		return res;
	}

	void write_priv_key_shard(utils::Buffer& out, const PrivKeyShard& shard)
	{
		const auto oldOutSize = out.size();
		out.resize(out.size() + SHARD_ID_MAX_SIZE + SHARD_VALUE_MAX_SIZE);
		auto* outData = out.data() + oldOutSize;

		utils::BigInt shardValUnderlying{};
		shard.first.Encode(outData, SHARD_ID_MAX_SIZE);
		shardValUnderlying = shard.second;
		shardValUnderlying.Encode(outData + SHARD_ID_MAX_SIZE, SHARD_VALUE_MAX_SIZE);
	}

	utils::Buffer priv_key_shard_to_bytes(const PrivKeyShard& shard)
	{
		utils::Buffer res{};
		write_priv_key_shard(res, shard);
		return res;
	}

	Seed sample_seed()
	{
		static thread_local auto dist =
			utils::Random<utils::BigInt>::get_dist_below(PubKey::order());

		return dist();
	}

	std::size_t get_seed_encoded_size()
	{
		return PubKey::order().MinEncodedSize();
	}

	utils::BytesView::iterator read_seed(Seed& out,
										 utils::BytesView::iterator it,
										 utils::BytesView::iterator end)
	{
		const auto size = std::min(
			PubKey::order().MinEncodedSize(),
			static_cast<std::size_t>(end - it)
		);

		out.Decode(std::to_address(it), size);
		return it + size;
	}

	Seed seed_from_bytes(utils::BytesView bytes)
	{
		Seed res{};
		read_seed(res, bytes.begin(), bytes.end());
		return res;
	}

	void write_seed(utils::Buffer& out, const Seed& seed)
	{
		const auto oldOutSize = out.size();
		const auto seedSize = PubKey::order().MinEncodedSize();
		out.resize(out.size() + seedSize);
		seed.Encode(out.data() + oldOutSize, seedSize);
	}

	utils::Buffer seed_to_bytes(const Seed& seed)
	{
		utils::Buffer res{};
		write_seed(res, seed);
		return res;
	}

	utils::BytesView::iterator read_evolution_offset(utils::BigInt& out,
		utils::BytesView::iterator it,
		utils::BytesView::iterator end)
	{
		return read_seed(out, it, end);
	}

	utils::BigInt evolution_offset_from_bytes(utils::BytesView bytes)
	{
		return seed_from_bytes(bytes);
	}

	void write_evolution_offset(utils::Buffer& out, const utils::BigInt& offset)
	{
		return write_seed(out, offset);
	}

	utils::Buffer evolution_offset_to_bytes(const utils::BigInt& offset)
	{
		return seed_to_bytes(offset);
	}
}

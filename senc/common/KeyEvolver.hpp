/*********************************************************************
 * \file   KeyEvolver.hpp
 * \brief  Header of `KeyEvolver` class.
 * 
 * \author aviad1b
 * \date   March 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "../utils/concepts.hpp"
#include "../utils/math.hpp"
#include "aliases.hpp"

namespace senc
{
	/**
	 * @class senc::KeyEvolver
	 * @brief Encapsulates key evolution logic.
	 */
	class KeyEvolver
	{
	public:
		using Self = KeyEvolver;

		/**
		 * @brief Constructs a new key evolver instance.
		 * @param seed Initial seed used for key evolution.
		 */
		KeyEvolver(Seed&& seed);

		/**
		 * @brief Copy constructor of key evolver.
		 */
		KeyEvolver(const Self&) = default;

		/**
		 * @brief Copy assignment operator of key evolver.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Move constructor of key evolver.
		 */
		KeyEvolver(Self&&) = default;

		/**
		 * @brief Move assignment operator of key evolver.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Evolves public keys and private key shards.
		 * @param args Public key and private key shard arguments (by ref).
		 */
		template <utils::OneOf<PubKey, PrivKeyShard>... Ts>
		void operator()(Ts&... args)
		{
			(apply_offset(args), ...);
			advance_offset();
		}

	private:
		// next offset for polynomial, computed pseudo-randomly each iteration
		utils::BigInt _offset;

		/**
		 * @brief Advances to next pseudo-random offset.
		 */
		void advance_offset();

		/**
		 * @brief Applies current offset on public key.
		 * @param key Public key (by ref).
		 */
		void apply_offset(PubKey& key);

		/**
		 * @brief Applies current offset on private key shard.
		 * @param shard Private key shard(by ref).
		 */
		void apply_offset(PrivKeyShard& shard);
	};
}

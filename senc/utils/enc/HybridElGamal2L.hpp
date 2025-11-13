/*********************************************************************
 * \file   HybridElGamal2L.hpp
 * \brief  Header of `enc::HybridElGamal2L` class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <cryptopp/integer.h>
#include <tuple>

#include "../concepts.hpp"
#include "../Group.hpp"
#include "general.hpp"

namespace senc::utils::enc
{
	/**
	 * @class senc::utils::enc::HybridElGamal2L
	 * @brief A `Asymmetric2L` schema which uses El-Gamal and a provided symmetric schema.
	 * @tparam G Group type for El-Gamal.
	 * @tparam S Symmetric (one-layer) encryption schema.
	 * @tparam KDF Key deriviation function (from two `G` elements to key for `S`).
	 */
	template <Group G, Symmetric1L S, ConstCallable<Key<S>, G, G> KDF>
	class HybridElGamal2L
	{
	public:
		using Self = HybridElGamal2L<G, S, KDF>;
		using Plaintext = Plaintext<S>;
		using Ciphertext = std::tuple<G, G, Ciphertext<S>>;
		using PubKey = G;
		using PrivKey = CryptoPP::Integer;

		/**
		 * @brief Constructs an instance with the given `S` and `KDF` instances.
		 * @param symmetricSchema `S` instance (symmetric schema)>
		 * @param kdf `KDF` instance (key deriviation function).
		 */
		HybridElGamal2L(S&& symmetricSchema, KDF&& kdf);

		/**
		 * @brief Encrypts a given plaintext.
		 * @param plaintext Plaintext to encrypt.
		 * @param pubKey1 Public key for first encryption layer.
		 * @param pubKey2 Public key for second encryption layer.
		 * @return Encrypted plaintext (ciphertext).
		 */
		Ciphertext encrypt(const Plaintext& plaintext, const PubKey& pubKey1, const PubKey& pubKey2) const;

		/**
		 * @brief Decrypts a given ciphertext.
		 * @param ciphertext Ciphertext to decrypt.
		 * @param privKey1 Private key for first encryption layer.
		 * @param privKey2 Private key for second encryption layer.
		 * @return Decrypted ciphertext (plaintext).
		 */
		Plaintext decrypt(const Ciphertext& ciphertext, const PrivKey& privKey1, const PrivKey& privKey2) const;

	private:
		S _symmetricSchema;
		KDF _kdf;
	};
}

#include "HybridElGamal2L_impl.hpp"

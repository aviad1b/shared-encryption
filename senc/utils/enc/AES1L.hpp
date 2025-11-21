/*********************************************************************
 * \file   AES1L.hpp
 * \brief  Header of `enc::AES1L` class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <cryptopp/secblock.h>
#include <cryptopp/osrng.h>
#include <tuple>

#include "../bytes.hpp"
#include "general.hpp"

namespace senc::utils::enc
{
	/**
	 * @class senc::utils::enc::AES1L
	 * @brief A `Symmetric1L` schema which uses AES.
	 */
	class AES1L
	{
	public:
		using Self = AES1L;
		using Plaintext = Buffer;
		using Ciphertext = std::tuple<CryptoPP::SecByteBlock, Buffer>;
		using Key = CryptoPP::SecByteBlock;

		static constexpr std::size_t KEY_SIZE = CryptoPP::AES::DEFAULT_KEYLENGTH;

		/**
		 * @brief Generates a key for AES one-layer schema.
		 */
		Key keygen();

		/**
		 * @brief Encrypts a plaintext using AES one-layer schema.
		 * @param plaintext Plaintext to encrypt.
		 * @param key Key to use for encryption.
		 * @return Encrypted plaintext (ciphertext).
		 */
		Ciphertext encrypt(const Plaintext& plaintext, const Key& key);

		/**
		 * @brief Decrypts a ciphertext using AES one-layer schema.
		 * @param ciphertext Ciphertext to decrypt.
		 * @param key Key to use for decryption.
		 * @return Decrypted ciphertext (plaintext).
		 */
		Plaintext decrypt(const Ciphertext& ciphertext, const Key& key);

	private:
		static CryptoPP::AutoSeededRandomPool _prng;
	};

	static_assert(Symmetric1L<AES1L>);
}

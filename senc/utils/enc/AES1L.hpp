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

namespace senc::utils::enc
{
	/**
	 * @class senc::utils::enc::AES1L
	 * @brief A `Symmetric1L` schema which uses AES.
	 */
	class AES1L
	{
	public:
		using Plaintext = CryptoPP::SecByteBlock;
		using Ciphertext = std::tuple<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock>;
		using Key = CryptoPP::SecByteBlock;

		/**
		 * @brief Generates a key for AES one-layer schema.
		 */
		Key keygen() const;

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
		CryptoPP::AutoSeededRandomPool _prng;
	};
}

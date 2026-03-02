/*********************************************************************
 * \file   aliases.hpp
 * \brief  Contains type aliases for both server and client.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../utils/enc/HybridElGamal2L.hpp"
#include "../utils/enc/ECHKDF2L.hpp"
#include "../utils/enc/AES1L.hpp"
#include "../utils/ECGroup.hpp"
#include "../utils/Shamir.hpp"
#include "../utils/uuid.hpp"
#include "sizes.hpp"
#include <string>

namespace senc
{
	/**
	 * @brief Identifier of non-owner encryption layer.
	 */
	constexpr auto REG_LAYER = 1;

	/**
	 * @brief Identifier of owner encryption layer.
	 */
	constexpr auto OWNER_LAYER = 2;

	/**
	 * @typedef senc::Schema
	 * @brief Encryption schema used by both client and server.
	 */
	using Schema = utils::enc::HybridElGamal2L<
		utils::ECGroup, utils::enc::AES1L, utils::enc::ECHKDF2L
	>;

	/**
	 * @typedef senc::Shamir
	 * @brief Holds implementations of Shamir utilities for threshold decryption.
	 */
	using Shamir = utils::ShamirHybridElGamal<
		utils::ECGroup, utils::enc::AES1L, utils::enc::ECHKDF2L,
		utils::BigInt
	>;

	/**
	 * @typedef senc::PubKey
	 * @brief Public key type (for encryption).
	 */
	using PubKey = utils::enc::PubKey<Schema>;

	/**
	 * @brief Reads a public key from bytes.
	 * @param out Variable to store read key to.
	 * @param it Iterator to read from.
	 * @param end Iterator to read untill.
	 * @return Iterator pointing to the end of read bytes.
	 */
	utils::Buffer::const_iterator read_pub_key(PubKey& out,
											   utils::Buffer::const_iterator it,
											   utils::Buffer::const_iterator end);

	/**
	 * @brief Reads a public key from a buffer of bytes.
	 * @param bytes Bytes to read key from.
	 * @return Read key.
	 */
	PubKey pub_key_from_bytes(const utils::Buffer& bytes);

	/**
	 * @brief Writes a public key into a buffer.
	 * @param out Buffer to write public key into.
	 * @param pubKey Public key to write.
	 */
	void write_pub_key(utils::Buffer& out, const PubKey& pubKey);

	/**
	 * @brief Serializes a public key into bytes.
	 * @param pubKey Public key to serialize.
	 * @return Buffer of bytes.
	 */
	utils::Buffer pub_key_to_bytes(const PubKey& pubKey);

	/**
	 * @typedef PrivKey
	 * @brief Private key type (for decryption).
	 */
	using PrivKey = utils::enc::PrivKey<Schema>;

	// server and client use plaintexts that are `utils::Buffer`.
	static_assert(std::same_as<utils::enc::Plaintext<Schema>, utils::Buffer>);

	/**
	 * @typedef Ciphertext
	 * @brief Ciphertext type of encryption schema.
	 */
	using Ciphertext = utils::enc::Ciphertext<Schema>;

	/**
	 * @typedef senc::PrivKeyShard
	 * @brief Shamir shard of a distributed private key.
	 */
	using PrivKeyShard = typename Shamir::Shard;

	/**
	 * @typedef senc::PrivKeyShardID
	 * @brief ID of private key Shamir shard (x value).
	 */
	using PrivKeyShardID = typename Shamir::ShardID;

	/**
	 * @typedef senc::PrivKeyShardValue
	 * @brief Value of private key Shamir shard (polynomial result, y value).
	 */
	using PrivKeyShardValue = typename Shamir::ShardValue;

	// this constant is true as shard IDs are currently sampled from [1,MAX_MEMBER_COUNT]
	// TODO: Make shard sampling use a constant from here for range
	constexpr std::size_t SHARD_ID_MAX_SIZE = sizeof(member_count_t);

	// this constant is true as shard values are always < modulus
	static const std::size_t SHARD_VALUE_MAX_SIZE = (PrivKeyShardValue::modulus() - 1).MinEncodedSize();

	/**
	 * @brief Reads a private key shard from bytes.
	 * @param out Variable to store read key shard to.
	 * @param it Iterator to read from.
	 * @param end Iterator to read untill.
	 * @return Iterator pointing to the end of read bytes.
	 */
	utils::Buffer::const_iterator read_priv_key_shard(PrivKeyShard& out,
													  utils::Buffer::const_iterator it,
													  utils::Buffer::const_iterator end);

	/**
	 * @brief Reads a private key shard from a buffer of bytes.
	 * @param bytes Bytes to read key from.
	 * @return Read key shard.
	 */
	PrivKeyShard priv_key_shard_from_bytes(const utils::Buffer& bytes);

	/**
	 * @brief Writes a private key shard into a buffer.
	 * @param out Buffer to write private key shard into.
	 * @param shard Private key shard to write.
	 */
	void write_priv_key_shard(utils::Buffer& out, const PrivKeyShard& shard);

	/**
	 * @brief Serializes a private key shard into bytes.
	 * @param shard Private key shard to serialize.
	 * @return Buffer of bytes.
	 */
	utils::Buffer priv_key_shard_to_bytes(const PrivKeyShard& shard);

	/**
	 * @typedef senc::DecryptionPart
	 * @brief Type of an unjoined decryption part computed using Shamir utilities and a private key Shamir shard.
	 */
	using DecryptionPart = typename Shamir::Part;

	/**
	 * @typedef senc::UserSetID
	 * @brief ID of userset.
	 */
	using UserSetID = utils::UUID;

	/**
	 * @typedef senc::OperationID
	 * @brief ID of decryption operation.
	 */
	using OperationID = utils::UUID;
}

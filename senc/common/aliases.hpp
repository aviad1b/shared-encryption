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

/*********************************************************************
 * \file   output.hpp
 * \brief  Header of client output utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#pragma once

#include "../../common/aliases.hpp"

namespace senc::debug_client::io
{
	/**
	 * @brief Prints public keys.
	 * @param regLayerPubKey1 Public key of non-owner layer to print.
	 * @param ownerLayerPubKey2 Public key of owner layer to print.
	 */
	void print_pub_keys(const PubKey& regLayerPubKey, const PubKey& ownerLayerPubKey);

	/**
	 * @brief Prints a private key shard of the non-owner layer.
	 * @param shard Private key shard to print.
	 */
	void print_reg_priv_key_shard(const PrivKeyShard& shard);
	
	/**
	 * @brief Prints an external private key shard of the owner layer.
	 * @param shard Private key shard to print.
	 */
	void print_owner_external_priv_key_shard(const PrivKeyShard& shard);

	/**
	 * @brief Prints an internal private key shard of the owner layer.
	 * @param shard Private key shard to print.
	 */
	void print_owner_internal_priv_key_shard(const PrivKeyShard& shard);

	/**
	 * @brief Prints ciphertext.
	 * @param ciphertext Ciphertext to print.
	 */
	void print_ciphertext(const Ciphertext& ciphertext);

	/**
	 * @brief Prints decryption part.
	 * @param decryptionPart Decryption part to print.
	 */
	void print_decryption_part(const DecryptionPart& decryptionPart);
}

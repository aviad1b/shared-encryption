/*********************************************************************
 * \file   output.hpp
 * \brief  Header of client output utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#pragma once

#include "../common/aliases.hpp"

namespace senc::client
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
	void print_reg_layer_priv_key_shard(const PrivKeyShard& shard);
	
	/**
	 * @brief Prints a private key shard of the owner layer.
	 * @param shard Private key shard to print.
	 */
	void print_owner_layer_priv_key_shard(const PrivKeyShard& shard);

	/**
	 * @brief Prints ciphertext.
	 * @param ciphertext Ciphertext to print.
	 */
	void print_ciphertext(const Ciphertext& ciphertext);
}

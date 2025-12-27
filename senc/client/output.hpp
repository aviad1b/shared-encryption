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
	 * @param pubKey1 Public key of first layer to print.
	 * @param pubKey2 Public key of second layer to print.
	 */
	void print_pub_keys(const PubKey& pubKey1, const PubKey& pubKey2);

	/**
	 * @brief Prints a private key shard of the first layer.
	 * @param shard Private key shard of the first layer to print.
	 */
	void print_priv_key1_shard(const PrivKeyShard& shard);
	
	/**
	 * @brief Prints a private key shard of the second layer.
	 * @param shard Private key shard of the second layer to print.
	 */
	void print_priv_key2_shard(const PrivKeyShard& shard);

	/**
	 * @brief Prints ciphertext.
	 * @param ciphertext Ciphertext to print.
	 */
	void print_ciphertext(const Ciphertext& ciphertext);
}

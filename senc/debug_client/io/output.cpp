/*********************************************************************
 * \file   output.cpp
 * \brief  Implementation of client output utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#include "output.hpp"

#include "../../utils/bytes.hpp"
#include <iostream>

using std::cout;
using std::endl;

namespace senc::client::io
{
	void print_pub_keys(const PubKey& regLayerPubKey, const PubKey& ownerLayerPubKey)
	{
		cout << "Encryption key: "
			 << utils::bytes_to_base64(pub_key_to_bytes(regLayerPubKey)) << endl
			 << utils::bytes_to_base64(pub_key_to_bytes(ownerLayerPubKey)) << endl;
	}

	// helper function, not accessed by main
	static void print_priv_key_any_shard(const char* label, const PrivKeyShard& shard)
	{
		cout << label << " decryption key shard: "
			 << utils::bytes_to_base64(priv_key_shard_to_bytes(shard))
			 << endl;
	}

	void print_reg_layer_priv_key_shard(const PrivKeyShard& shard)
	{
		print_priv_key_any_shard("Non-owner", shard);
	}

	void print_owner_layer_priv_key_shard(const PrivKeyShard& shard)
	{
		print_priv_key_any_shard("Owner", shard);
	}

	void print_ciphertext(const Ciphertext& ciphertext)
	{
		const auto& [c1, c2, c3] = ciphertext;
		const auto& [c3a, c3b] = c3;

		cout << utils::bytes_to_base64(c1.encode()) << endl
			 << utils::bytes_to_base64(c2.encode()) << endl
			 << utils::bytes_to_base64(c3a) << endl
			 << utils::bytes_to_base64(c3b) << endl;
	}

	void print_decryption_part(const DecryptionPart& decryptionPart)
	{
		cout << utils::bytes_to_base64(decryptionPart.encode()) << endl;
	}
}

/*********************************************************************
 * \file   output.cpp
 * \brief  Implementation of client output utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#include "output.hpp"

#include "../utils/bytes.hpp"
#include <iostream>

using std::cout;
using std::endl;

namespace senc
{
	void print_pub_keys(const PubKey& pubKey1, const PubKey& pubKey2)
	{
		cout << "First public key:" << endl << utils::bytes_to_base64(pubKey1.to_bytes()) << endl << endl;
		cout << "Second public key:" << endl << utils::bytes_to_base64(pubKey2.to_bytes()) << endl << endl;
	}

	// helper function, not accessed by main
	static void print_priv_key_any_shard(const char* label, const PrivKeyShard& shard)
	{
		cout << label << "decryption key shard: (" << shard.first << "," << shard.second << ")" << endl << endl;
	}

	void print_priv_key1_shard(const PrivKeyShard& shard)
	{
		print_priv_key_any_shard("Non-owner", shard);
	}

	void print_priv_key2_shard(const PrivKeyShard& shard)
	{
		print_priv_key_any_shard("Owner", shard);
	}
}

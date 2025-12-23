/*********************************************************************
 * \file   output.hpp
 * \brief  Header of client output utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#pragma once

#include "../common/aliases.hpp"

namespace senc
{
	void print_pub_keys(const PubKey& pubKey1, const PubKey& pubKey2);

	void print_priv_key1_shard(const PrivKeyShard& shard);
	
	void print_priv_key2_shard(const PrivKeyShard& shard);
}

/*********************************************************************
 * \file   KeyEvolver.cpp
 * \brief  Implementation of `KeyEvolver` class.
 * 
 * \author aviad1b
 * \date   March 2026, Adar 5786
 *********************************************************************/

#include "KeyEvolver.hpp"

#include "../utils/pseudo_random.hpp"
#include "../utils/Random.hpp"

namespace senc
{
	KeyEvolver::KeyEvolver(Seed&& seed) : _offset(std::move(seed)) { }

	void KeyEvolver::advance_offset()
	{
		this->_offset = utils::pseudo_random(_offset, PubKey::order().MinEncodedSize());
	}

	void KeyEvolver::apply_offset(PubKey& key)
	{
		// key is g^s. change to g^(s+offset)
		key *= PubKey::generator().pow(this->_offset);
	}

	void KeyEvolver::apply_offset(PrivKeyShard& shard)
	{
		// shard is (x,y). change to (x,y+offset)
		shard.second += this->_offset;
	}
}

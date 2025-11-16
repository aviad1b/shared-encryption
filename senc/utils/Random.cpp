/*********************************************************************
 * \file   Random.cpp
 * \brief  Contains implementation of utilities for Random class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Random.hpp"

namespace senc::utils
{
	BigIntUnderlyingDist::BigIntUnderlyingDist(const BigInt& min, const BigInt& max)
		: _min(min), _max(max) { }

	BigInt BigIntUnderlyingDist::operator()(CryptoPP::RandomNumberGenerator& engine) const
	{
		BigInt res;
		res.Randomize(
			engine, this->_min, this->_max,
			BigInt::RandomNumberType::ANY
		);
		return res;
	}
}

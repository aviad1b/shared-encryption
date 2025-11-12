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
	CryptoUnderlyingDist::CryptoUnderlyingDist(const CryptoPP::Integer& min, const CryptoPP::Integer& max)
		: _min(min), _max(max) { }

	CryptoPP::Integer CryptoUnderlyingDist::operator()(CryptoPP::RandomNumberGenerator& engine) const
	{
		CryptoPP::Integer res;
		res.Randomize(
			engine, this->_min, this->_max,
			CryptoPP::Integer::RandomNumberType::ANY
		);
		return res;
	}
}

/*********************************************************************
 * \file   ECHKDF1L.cpp
 * \brief  Implementation of ECHKDF1L class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "ECHKDF1L.hpp"

namespace senc::utils::enc
{
	ECHKDF1L::ECHKDF1L() : Self(DEFAULT_IKM_SIZE) { }

	ECHKDF1L::ECHKDF1L(std::size_t ikmSize)
		: Self(ikmSize, DEFAULT_SALT) { }

	ECHKDF1L::ECHKDF1L(const Buffer& salt)
		: Self(DEFAULT_IKM_SIZE, salt) { }

	ECHKDF1L::ECHKDF1L(std::size_t ikmSize, const Buffer& salt)
		: _ikmSize(ikmSize), _salt(salt) { }

	AES1L::Key ECHKDF1L::operator()(const ECGroup& elem) const
	{
		const auto& num = elem.x();

		const std::size_t size = num.MinEncodedSize();

		// make SIZE-byte input keying material for HKDF out of num:
		Buffer ikm(_ikmSize);
		num.Encode(ikm.data(), size);

		AES1L::Key key(AES1L::KEY_SIZE);
		_hkdf.DeriveKey(
			key.data(), key.size(),
			ikm.data(), ikm.size(),
			_salt.data(), _salt.size(),
			nullptr, 0
		);

		return key;
	}
}

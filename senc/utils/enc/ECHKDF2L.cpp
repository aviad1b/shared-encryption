/*********************************************************************
 * \file   ECHKDF2L.cpp
 * \brief  Implementation of ECHKDF2L class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "ECHKDF2L.hpp"

namespace senc::utils::enc
{
	ECHKDF2L::ECHKDF2L() : Self(DEFAULT_IKM_EACH_SIZE) { }

	ECHKDF2L::ECHKDF2L(std::size_t ikmEachSize)
		: Self(ikmEachSize, DEFAULT_SALT) { }

	ECHKDF2L::ECHKDF2L(const Buffer& salt)
		: Self(DEFAULT_IKM_EACH_SIZE, salt) { }

	ECHKDF2L::ECHKDF2L(std::size_t ikmEachSize, const Buffer& salt)
		: _ikmEachSize(ikmEachSize), _salt(salt) { }

	AES1L::Key senc::utils::enc::ECHKDF2L::operator()(const ECGroup& a, const ECGroup& b) const
	{
		const auto& aNum = a.x();
		const auto& bNum = b.x();

		const std::size_t aSize = aNum.MinEncodedSize();
		const std::size_t bSize = bNum.MinEncodedSize();

		// make (EACH_SIZE * 2)-byte input keying material for HKDF out of aNum and bNum:
		Buffer ikm(_ikmEachSize * 2);
		aNum.Encode(ikm.data() + (_ikmEachSize - aSize), aSize);
		bNum.Encode(ikm.data() + _ikmEachSize + (_ikmEachSize - bSize), bSize);

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

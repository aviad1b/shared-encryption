/*********************************************************************
 * \file   ECHKDF1L.hpp
 * \brief  Header of ECHKDF1L class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../ECGroup.hpp"
#include "AES1L.hpp"

namespace senc::utils::enc
{
	/**
	 * @class senc::utils::enc::ECHKDF1L
	 * @brief Default-constructible HKDF functor for Elliptic Curve elements (one-layer).
	 */
	class ECHKDF1L
	{
	public:
		using Self = ECHKDF1L;

		/**
		 * @brief Constructs an ECHKDF1L instance with default parameters.
		 */
		ECHKDF1L();

		/**
		 * @brief Constructs an ECHKDF1L instance with given allocated size for group element in IKM.
		 * @param ikmSize Size to use in input keying material group element.
		 */
		ECHKDF1L(std::size_t ikmSize);

		/**
		 * @brief Constructs an ECHKDF1L with given salt value.
		 * @param salt Salt value.
		 */
		ECHKDF1L(const Buffer& salt);

		/**
		 * @brief Constructs an ECHKDF1L instance with given parameter values.
		 * @param ikmSize Size to use in input keying material for group element.
		 * @param salt Salt value.
		 */
		ECHKDF1L(std::size_t ikmSize, const Buffer& salt);

		/**
		 * @brief Derives an AES1L key from an ECGroup element.
		 * @param elem ECGroup element.
		 * @return Derives AES1L key.
		 */
		AES1L::Key operator()(const ECGroup& elem) const;

	private:
		std::size_t _ikmSize;
		Buffer _salt;

		CryptoPP::HKDF<CryptoPP::SHA256> _hkdf;

		static constexpr std::size_t DEFAULT_IKM_SIZE = 64; // default ikmSize is 64 (64-byte IKM)
		static constexpr std::initializer_list<byte> DEFAULT_SALT = { 4, 3, 5 }; // default salt value
	};
}

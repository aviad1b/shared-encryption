/*********************************************************************
 * \file   PBKDF2.hpp
 * \brief  Header of PBKDF2 class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include <cryptopp/pwdbased.h>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <string>
#include <array>
#include "../bytes.hpp"
#include "general.hpp"

namespace senc::utils::pwd
{
	/**
	 * @class senc::utils::pwd::PBKDF2
	 * @brief Password-Based Key Derivation Function 2 implementation of `senc::utils::pwrd::PwdHash`.
	 */
	class PBKDF2
	{
	public:
		using Self = PBKDF2;
		using Password = std::string;
		using Salt = std::array<byte, 16>;
		using Output = std::array<byte, 32>;

		/**
		 * @brief Constructs a PBKDF2 password hash schema.
		 * @param iterations Amount of iterations to use for PBKDF2 hash.
		 */
		PBKDF2(std::size_t iterations = 100000);

		/**
		 * @brief Generates salt for password hashing.
		 * @return Generated salt.
		 */
		Salt generate_salt();

		/**
		 * @brief Hashes a given password using a given salt.
		 * @param password Password to hash.
		 * @param salt Salt to use for hash.
		 * @return Password hash.
		 */
		Output hash(const Password& password, const Salt& salt);

	private:
		CryptoPP::AutoSeededRandomPool _saltRng;
		CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> _pbkdf2;
		std::size_t _iterations;
	};
}

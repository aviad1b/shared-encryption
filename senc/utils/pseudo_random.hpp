/*********************************************************************
 * \file   pseudo_random.hpp
 * \brief  Contains pseudo-random computation utilities.
 * 
 * \author aviad1b
 * \date   March 2026, Adar 5786
 *********************************************************************/

#pragma once

#include <cryptopp/secblock.h>
#include <cryptopp/filters.h>
#include <cryptopp/sha.h>
#include "bytes.hpp"
#include "math.hpp"

namespace senc::utils
{
    /**
     * @brief Computes a pseudo-random integral value based on input.
     * @param input
     * @return Pseudo-random output based on `input`.
     */
	template <std::integral T>
	inline T pseudo_random(T input)
	{
        CryptoPP::SHA256 hash{};
        CryptoPP::SecByteBlock digest(hash.DigestSize());

        hash.Update(reinterpret_cast<const byte*>(&input), sizeof(T));
        hash.Final(digest);

        T res{};
        std::memcpy(&res, digest.data(), sizeof(T));
        return res;
	}

    /**
     * @brief Computes a pseudo-random `BigInt` value based on input.
     * @param input
     * @param size Maximum size for input & output.
     * @return Pseudo-random output based on `input`.
     */
    inline BigInt pseudo_random(const BigInt& input, std::size_t size)
    {
        CryptoPP::SHA256 hash{};
        CryptoPP::SecByteBlock digest(hash.DigestSize());

        Buffer buff(size, 0);
        input.Encode(buff.data(), size);
        hash.Update(buff.data(), size);
        hash.Final(digest);

        BigInt res;
        res.Decode(digest.data(), size);
        return res;
    }
}

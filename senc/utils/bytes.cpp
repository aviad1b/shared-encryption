/*********************************************************************
 * \file   bytes.cpp
 * \brief  Implementation of byte conversion utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "bytes.hpp"

#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/queue.h>

#include "Random.hpp"

namespace senc::utils
{
	Buffer random_bytes(std::size_t count)
	{
		auto dist = Random<byte>::get_dist();
		Buffer res(count);

		for (auto& b : res)
			b = dist();

		return res;
	}

	Buffer bytes_from_base64(const std::string& base64)
	{
		Buffer res;
		CryptoPP::StringSource(
			base64,
			true,
			new CryptoPP::Base64Decoder(
				new CryptoPP::VectorSink(res)
			)
		);
		return res;
	}
}

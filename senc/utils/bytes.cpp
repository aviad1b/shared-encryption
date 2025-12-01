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

namespace senc::utils
{
	std::string senc::utils::bytes_to_base64(const Buffer& bytes)
	{
		std::string res;
		CryptoPP::StringSource(
			bytes.data(),
			bytes.size(),
			true,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink(res),
				false // insertLineBreaks=false
			)
		);
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

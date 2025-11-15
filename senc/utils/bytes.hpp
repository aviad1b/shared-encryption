/*********************************************************************
 * \file   bytes.hpp
 * \brief  Contains definitions related to binary data.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <cryptopp/config_int.h>
#include <vector>

namespace senc::utils
{
	/**
	 * @typedef senc::utils::byte
	 * @brief Single byte value premitive.
	 */
	using byte = CryptoPP::byte;

	/**
	 * @typedef senc::utils::Buffer
	 * @brief Stores sequential binary data.
	 */
	using Buffer = std::vector<byte>;
}

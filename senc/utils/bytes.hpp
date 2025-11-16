/*********************************************************************
 * \file   bytes.hpp
 * \brief  Contains definitions related to binary data.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <cryptopp/config_int.h>
#include <concepts>
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

	/**
	 * @concept senc::utils::HasByteData
	 * @brief Looks for a typename which stores sequential data of bytes.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasByteData = requires(const Self self)
	{
		{ self.data() } -> std::convertible_to<const byte*>;
		{ self.size() } -> std::convertible_to<std::size_t>;
	};

	/**
	 * @concept senc::utils::HasMutableByteData
	 * @brief Looks for a typename which stores sequential data of bytes that can be modified.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasMutableByteData = requires(Self self)
	{
		{ self.data() } -> std::convertible_to<byte*>;
		{ self.size() } -> std::convertible_to<std::size_t>;
	};

	/**
	 * @concept senc::utils::HasToBytes
	 * @brief Looks for a typename which has a `to_bytes` method for binary representation.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasToBytes = requires(const Self self)
	{
		{ self.to_bytes() } -> std::convertible_to<Buffer>;
	};

	/**
	 * @concept senc::utils::HasFromBytes
	 * @brief Looks for a typename which has a `from_bytes` method for binary parsing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasFromBytes = requires(const Buffer bytes)
	{
		{ Self::from_bytes(bytes) } -> std::convertible_to<Self>;
	};
}

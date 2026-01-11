/*********************************************************************
 * \file   bytes.hpp
 * \brief  Contains definitions related to binary data.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

// this include is needed because CryptoPP uses WinAPI
#include "../utils/winapi_patch.hpp"

#include <cryptopp/config_int.h>
#include <vector>
#include <string>
#include <ranges>
#include <bit>

#include "concepts.hpp"

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

	/**
	 * @concept senc::utils::HasFixedBytesSize
	 * @brief Looks for a typename which has a `bytes_size` method, telling its (serialized) bytes size.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasFixedBytesSize = requires
	{
		{ Self::bytes_size() } -> std::convertible_to<std::size_t>;
	};

	/**
	 * @brief Generates a buffer of random bytes.
	 * @param count Amount of bytes to generate (buffer size).
	 * @return A buffer of `count` random bytes.
	 */
	Buffer random_bytes(std::size_t count);

	/**
	 * @brief Converts a fundamental/enum value to a buffer of bytes.
	 * @param value Value to convert to bytes.
	 * @return Bytes representation of `value`.
	 */
	template <typename T>
	requires std::is_fundamental_v<T> || std::is_enum_v<T>
	Buffer to_bytes(T value);

	/**
	 * @brief Parses a fundamental/enum value from a buffer of bytes.
	 * @param bytes Buffer of bytes to parse value from.
	 * @return Parsed value.
	 */
	template <typename T>
	requires std::is_fundamental_v<T> || std::is_enum_v<T>
	T from_bytes(const Buffer& bytes);

	/**
	 * @brief Converts a string value to a buffer of bytes.
	 * @param str String to convert to bytes.
	 * @return Bytes representation of `str`.
	 */
	template <StringType T>
	Buffer to_bytes(const T& str);

	/**
	 * @brief Parses a string value from a buffer of bytes.
	 * @param bytes Buffer of bytes to parse value from.
	 * @return Parsed string.
	 */
	template <StringType T>
	T from_bytes(const Buffer& bytes);

	/**
	 * @brief Converts a `HasToBytes` object to a buffer of bytes.
	 * @param obj Object to convert to bytes.
	 * @return Bytes representation of `obj`.
	 */
	template <HasToBytes T>
	Buffer to_bytes(const T& obj);

	/**
	 * @brief Parses a `HasFromBytes` object from a buffer of bytes.
	 * @param bytes Buffer of bytes to parse object from.
	 * @return Parsed object.
	 */
	template <HasFromBytes T>
	T from_bytes(const Buffer& bytes);

	/**
	 * @concept senc::utils::ByteConvertible
	 * @brief Looks for a typename that can be converted to bytes using `to_bytes`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ByteConvertible = requires(const Self& self)
	{
		{ ::senc::utils::to_bytes(self) } -> std::convertible_to<Buffer>;
	};

	/**
	 * @concept senc::utils::ByteParsable
	 * @brief Looks for a typename that can be parsed from bytes using `from_bytes`.
	 * @tparam Self Examied typename.
	 */
	template <typename Self>
	concept ByteParsable = requires(const Buffer& bytes)
	{
		{ ::senc::utils::from_bytes<Self>(bytes) } -> std::convertible_to<Self>;
	};

	/**
	 * @brief Converts a buffer of bytes to base64 representation.
	 * @param bytes Buffer of bytes to encode in base64.
	 * @return Base64 string.
	 */
	std::string bytes_to_base64(const HasByteData auto& bytes);

	template <std::ranges::input_range R>
	requires (std::same_as<std::ranges::range_value_t<R>, byte> && !HasByteData<R>)
	std::string bytes_to_base64(R&& rng);

	/**
	 * @brief Converts base64 representation to a buffer of bytes.
	 * @param base64 Base64 string to decode into bytes.
	 * @return Buffer of bytes.
	 */
	Buffer bytes_from_base64(const std::string& base64);

	/**
	 * @brief Writes a primitive value to an array of bytes.
	 * @tparam endianess Endianess to use.
	 * @param bytes Array of bytes to append to (by ref).
	 * @param value Value to write.
	 */
	template <std::endian endianess = std::endian::big>
	void write_bytes(Buffer& bytes, auto value)
	requires (std::is_fundamental_v<std::remove_cvref_t<decltype(value)>> ||
		std::is_enum_v<std::remove_cvref_t<decltype(value)>>);

	/**
	 * @brief Reads a (null-terminated) string from bytes.
	 * @tparam endianess Endianess to use.
	 * @param it Bytes iterator to start reading from.
	 * @param end Bytes iterator to data end.
	 * @param out Variable to store read value into.
	 * @return Iterator pointing to after read value.
	 */
	template <std::endian endianess = std::endian::big>
	Buffer::iterator read_bytes(Buffer::iterator it, Buffer::iterator end, std::string& out);

	/**
	 * @brief Reads a fundamental/enum value from bytes.
	 * @tparam endianess Endianess to use.
	 * @param it Bytes iterator to start reading from.
	 * @param end Bytes iterator to data end.
	 * @param out Variable to store read value into.
	 * @return Iterator pointing to after read value.
	 */
	template <std::endian endianess = std::endian::big>
	Buffer::iterator read_bytes(Buffer::iterator it, Buffer::iterator end, auto& out)
	requires (std::is_fundamental_v<std::remove_cvref_t<decltype(out)>> ||
		std::is_enum_v<std::remove_cvref_t<decltype(out)>>);
}

#include "bytes_impl.hpp"

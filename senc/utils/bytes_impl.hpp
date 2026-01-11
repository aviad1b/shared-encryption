/*********************************************************************
 * \file   bytes_impl.hpp
 * \brief  Template implementation of byte conversion utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "bytes.hpp"

// this include is needed because CryptoPP uses WinAPI
#include "../utils/winapi_patch.hpp"

#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/queue.h>
#include <cstring>

#include "Exception.hpp"

namespace senc::utils
{
	template <typename T>
	requires std::is_fundamental_v<T> || std::is_enum_v<T>
	inline Buffer to_bytes(T value)
	{
		const byte* data = static_cast<const byte*>(&value);
		return Buffer(data, data + sizeof(T));
	}

	template <typename T>
	requires std::is_fundamental_v<T> || std::is_enum_v<T>
	inline T from_bytes(const Buffer& bytes)
	{
		if (bytes.size() != sizeof(T))
			throw Exception("Bad size for parse");
		return *static_cast<const T*>(bytes.data());
	}

	template <std::ranges::input_range R>
	requires (std::same_as<std::ranges::range_value_t<R>, byte> && !HasByteData<R>)
	std::string bytes_to_base64(R&& rng)
	{
		return bytes_to_base64(Buffer(std::ranges::begin(rng), std::ranges::end(rng)));
	}

	template <StringType T>
	inline Buffer to_bytes(const T& str)
	{
		return Buffer(
			static_cast<const byte*>(str.c_str()),
			static_cast<const byte*>(str.c_str() + str.length() + 1)
		);
	}

	template <StringType T>
	inline T from_bytes(const Buffer& bytes)
	{
		using C = typename T::value_type;
		const C nullchr = 0;
		if (0 != std::memcmp(bytes.data() - sizeof(C), &nullchr, sizeof(C)))
			throw Exception("Bad string (no null-termination)");
		return T(static_cast<const C*>(bytes.data()));
	}

	template <HasToBytes T>
	inline Buffer to_bytes(const T& obj)
	{
		return obj.to_bytes();
	}

	template <HasFromBytes T>
	inline T from_bytes(const Buffer& bytes)
	{
		return T::from_bytes(bytes);
	}

	inline std::string bytes_to_base64(const HasByteData auto& bytes)
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

	template <std::endian endianess>
	void write_bytes(Buffer& bytes, const std::string& value)
	{
		bytes.insert(
			bytes.end(),
			value.begin(),
			value.end()
		);
	}

	template <std::endian endianess>
	void write_bytes(Buffer& bytes, auto value)
	requires (std::is_fundamental_v<std::remove_cvref_t<decltype(value)>> ||
		std::is_enum_v<std::remove_cvref_t<decltype(value)>>)
	{
		auto oldEnd = bytes.end();
		bytes.insert(
			oldEnd,
			reinterpret_cast<const byte*>(&value),
			reinterpret_cast<const byte*>(&value + 1)
		);

		if constexpr (std::endian::native != endianess)
			std::reverse(oldEnd, bytes.end());
	}

	template <std::endian endianess>
	void write_bytes(Buffer& bytes, const auto& value)
	requires HasByteData<std::remove_cvref_t<decltype(value)>>
	{
		bytes.insert(bytes.end(), value.data(), value.data() + value.size());
	}

	template <std::endian endianess>
	Buffer::iterator read_bytes(std::string& out, Buffer::iterator it, Buffer::iterator end)
	{
		const char* p = reinterpret_cast<const char*>(std::to_address(it));
		const char* pEnd = reinterpret_cast<const char*>(std::to_address(end));
		const char* null = std::find(p, pEnd, 0);

		// if has null termination, simply assign as string;
		// otherwise, read everything untill end
		out = std::string(p, std::min(null, pEnd));

		it += out.length() + 1; // including null-termination

		return it;
	}

	template <std::endian endianess>
	Buffer::iterator read_bytes(auto& out, Buffer::iterator it, Buffer::iterator end)
	requires (std::is_fundamental_v<std::remove_cvref_t<decltype(out)>> ||
		std::is_enum_v<std::remove_cvref_t<decltype(out)>>)
	{
		using T = std::remove_cvref_t<decltype(out)>;
		const std::size_t availableData = end - it;
		const std::size_t readSize = std::min(sizeof(T), availableData);
		std::memcpy(
			&out,
			std::to_address(it),
			readSize
		);

		if constexpr (std::endian::native != endianess)
			std::reverse(
				reinterpret_cast<byte*>(&out),
				reinterpret_cast<byte*>(&out + 1)
			);

		it += readSize;

		return it;
	}

	template <std::endian endianess>
	Buffer::iterator read_bytes(auto& out, Buffer::iterator it, Buffer::iterator end)
	requires HasMutableByteData<std::remove_cvref_t<decltype(out)>>
	{
		const std::size_t size = std::min(std::distance(end, it), out.size());
		std::memcpy(out.data(), std::to_address(it), size);
		return it + size;
	}
}

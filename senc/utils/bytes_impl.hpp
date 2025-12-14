/*********************************************************************
 * \file   bytes_impl.hpp
 * \brief  Template implementation of byte conversion utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "bytes.hpp"

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

	inline std::string bytes_to_base64(const Buffer& bytes)
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
}

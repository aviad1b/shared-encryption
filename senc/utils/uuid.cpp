/*********************************************************************
 * \file   uuid.cpp
 * \brief  Implementation of UUID utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "uuid.hpp"

#include <cstdint>

namespace senc::utils
{
	UUID::UUID() : _str("00000000-0000-0000-0000-000000000000") { }

	UUID::UUID(const char* value) : Self(std::string(value)) { }

	UUID::UUID(const std::string& value) : Self(std::string(value)) { }

	UUID::UUID(std::string&& value) : _str(std::move(value))
	{
		Underlying underlying{};
		if (RPC_S_INVALID_STRING_UUID == UuidFromStringA((RPC_CSTR)_str.c_str(), &underlying))
			throw UUIDException("Bad UUID", value);

		bytes_from_underlying(this->_bytes, underlying);
	}

	UUID::Self UUID::generate()
	{
		Underlying uuid{};
		UuidCreate(&uuid);
		return Self(uuid);
	}

	bool UUID::operator==(const Self& other) const noexcept
	{
		return this->_bytes == other._bytes;
	}

	const std::string& UUID::to_string() const noexcept
	{
		return this->_str;
	}

	Buffer UUID::to_bytes() const
	{
		return Buffer(
			this->_bytes.data(),
			this->_bytes.data() + this->_bytes.size()
		);
	}

	UUID::Self UUID::from_bytes(const Buffer& bytes)
	{
		// parsing from bytes using RFC-4122

		Underlying value{};

		value.Data1 = (static_cast<std::uint32_t>(bytes[0]) << 24) |
			(static_cast<uint32_t>(bytes[1]) << 16) |
			(static_cast<uint32_t>(bytes[2]) << 8) |
			(static_cast<uint32_t>(bytes[3]));

		value.Data2 = (static_cast<uint16_t>(bytes[4]) << 8) |
			(static_cast<uint16_t>(bytes[5]));

		value.Data3 = (static_cast<uint16_t>(bytes[6]) << 8) |
			(static_cast<uint16_t>(bytes[7]));

		for (std::size_t i = 0; i < 8; i++)
			value.Data4[i] = bytes[8 + i];

		return Self(value);
	}

	UUID::UUID(const Underlying& value)
	{
		bytes_from_underlying(this->_bytes, value);

		// get string representation
		char* str = nullptr;
		if (RPC_S_OUT_OF_MEMORY == UuidToStringA(&value, (RPC_CSTR*)&str))
			throw std::bad_alloc{};
		this->_str = str;
		RpcStringFreeA((RPC_CSTR*)&str);
	}

	void UUID::bytes_from_underlying(std::array<byte, 16>& out, const Underlying& underlying)
	{
		// get bytes (RFC-4122 order)

		out[0] = (underlying.Data1 >> 24) & 0xFF;
		out[1] = (underlying.Data1 >> 16) & 0xFF;
		out[2] = (underlying.Data1 >> 8) & 0xFF;
		out[3] = (underlying.Data1) & 0xFF;

		out[4] = (underlying.Data2 >> 8) & 0xFF;
		out[5] = (underlying.Data2) & 0xFF;

		out[6] = (underlying.Data3 >> 8) & 0xFF;
		out[7] = (underlying.Data3) & 0xFF;

		for (std::size_t i = 0; i < 8; i++)
			out[8 + i] = underlying.Data4[i];
	}

	std::ostream& operator<<(std::ostream& os, const UUID& uuid)
	{
		return os << uuid._str;
	}
}

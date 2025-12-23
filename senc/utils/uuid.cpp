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
	UUID::UUID(const char* value) : Self(std::string(value)) { }

	UUID::UUID(const std::string& value)
	{
		Underlying underlying{};
		if (RPC_S_INVALID_STRING_UUID == UuidFromStringA((RPC_CSTR)value.c_str(), &underlying))
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

	std::string UUID::to_string() const
	{
		Underlying underlying{};
		char* str = nullptr;

		underlying_from_bytes(underlying, this->_bytes);
		if (RPC_S_OUT_OF_MEMORY == UuidToStringA(&underlying, (RPC_CSTR*)&str))
			throw std::bad_alloc{};

		std::string res = str;
		RpcStringFreeA((RPC_CSTR*)&str);
		return res;
	}

	std::size_t UUID::hash() const noexcept
	{
		return Hash<std::string_view>()(
			std::string_view(reinterpret_cast<const char*>(data()), size())
		);
	}

	byte* UUID::data()
	{
		return this->_bytes.data();
	}

	const byte* UUID::data() const
	{
		return this->_bytes.data();
	}

	UUID::UUID(const Underlying& value)
	{
		bytes_from_underlying(this->_bytes, value);
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

	void UUID::underlying_from_bytes(Underlying& out, const std::array<byte, 16>& bytes)
	{
		out.Data1 = (static_cast<std::uint32_t>(bytes[0]) << 24) |
			(static_cast<std::uint32_t>(bytes[1]) << 16) |
			(static_cast<std::uint32_t>(bytes[2]) << 8) |
			(static_cast<std::uint32_t>(bytes[3]));

		out.Data2 = (static_cast<std::uint32_t>(bytes[4]) << 8) |
			(static_cast<std::uint32_t>(bytes[5]));

		out.Data3 = (static_cast<std::uint32_t>(bytes[6]) << 8) |
			(static_cast<std::uint32_t>(bytes[7]));

		for (std::size_t i = 0; i < 8; i++)
			out.Data4[i] = bytes[8 + i];
	}

	std::ostream& operator<<(std::ostream& os, const UUID& uuid)
	{
		return os << uuid.to_string();
	}
}

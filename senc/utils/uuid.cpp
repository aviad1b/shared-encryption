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

	UUID::UUID(const std::string& value) : _str(value)
	{
		if (RPC_S_INVALID_STRING_UUID == UuidFromStringA((RPC_CSTR)value.c_str(), &_value))
			throw UUIDException("Bad UUID", value);
	}

	UUID::Self UUID::generate()
	{
		Underlying uuid{};
		UuidCreate(&uuid);
		return Self(uuid);
	}

	bool UUID::operator==(const Self& other) const noexcept
	{
		RPC_STATUS status = RPC_S_OK;
		return static_cast<bool>(UuidEqual(
			const_cast<Underlying*>(&this->_value),
			const_cast<Underlying*>(&other._value),
			&status
		));
	}

	const std::string& UUID::to_string() const noexcept
	{
		return this->_str;
	}

	Buffer UUID::to_bytes() const
	{
		// converting to bytes using RFC-4122

		Buffer res(bytes_size());

		res[0] = (this->_value.Data1 >> 24) & 0xFF;
		res[1] = (this->_value.Data1 >> 16) & 0xFF;
		res[2] = (this->_value.Data1 >> 8) & 0xFF;
		res[3] = (this->_value.Data1) & 0xFF;

		res[4] = (this->_value.Data2 >> 8) & 0xFF;
		res[5] = (this->_value.Data2) & 0xFF;

		res[6] = (this->_value.Data3 >> 8) & 0xFF;
		res[7] = (this->_value.Data3) & 0xFF;

		for (std::size_t i = 0; i < 8; i++)
			res[8 + i] = this->_value.Data4[i];

		return res;
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

	UUID::UUID(const Underlying& value) : _value(value)
	{
		char* str = nullptr;
		if (RPC_S_OUT_OF_MEMORY == UuidToStringA(&value, (RPC_CSTR*)&str))
			throw std::bad_alloc{};
		this->_str = str;
		RpcStringFreeA((RPC_CSTR*)&str);
	}

	std::ostream& operator<<(std::ostream& os, const UUID& uuid)
	{
		return os << uuid._str;
	}
}

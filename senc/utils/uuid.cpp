/*********************************************************************
 * \file   uuid.cpp
 * \brief  Implementation of UUID utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "uuid.hpp"

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
		::UUID uuid{};
		UuidCreate(&uuid);
		return Self(uuid);
	}

	bool UUID::operator==(const Self& other) const noexcept
	{
		RPC_STATUS status = RPC_S_OK;
		return static_cast<bool>(UuidEqual(
			const_cast<::UUID*>(&this->_value),
			const_cast<::UUID*>(&other._value),
			&status
		));
	}

	const std::string& UUID::to_string() const noexcept
	{
		return this->_str;
	}

	UUID::UUID(const ::UUID& value) : _value(value)
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

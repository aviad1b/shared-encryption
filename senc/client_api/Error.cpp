/*********************************************************************
 * \file   Error.cpp
 * \brief  Implementation of `Error` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "Error.hpp"

namespace senc::clientapi
{
	Error::Self Error::_ALLOCATION = Error::make_instance("Allocation Error");

	Error::Self* Error::ALLOCATION = &_ALLOCATION;

	Error::Self Error::make_instance(std::string&& msg) noexcept
	{
		return Self(false, std::move(msg));
	}

	ValueOrError* Error::new_instance(std::string&& msg) noexcept
	{
		try { return new Self(true, std::move(msg)); }
		catch (const std::bad_alloc&) { return Error::ALLOCATION; }
		catch (const std::exception& e) { return Error::new_instance(e.what()); }
	}

	bool Error::has_error() const
	{
		return true;
	}

	const char* Error::what() const
	{
		return _msg.c_str();
	}

	Error::Error(bool isAllocated, std::string&& msg) noexcept
		: Base(isAllocated), _msg(std::move(msg)) { }
}

/*********************************************************************
 * \file   Exception.cpp
 * \brief  Implementation of base exception class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Exception.hpp"

namespace senc::utils
{
	Exception::Exception(const std::string& msg) : _msg(msg) { }

	Exception::Exception(std::string&& msg) : _msg(std::move(msg)) { }

	Exception::Exception(const std::string& msg, const std::string& info)
		: Self(std::string(msg), info) { }

	Exception::Exception(std::string&& msg, const std::string& info)
		: Self(std::move(msg))
	{
		if (!info.empty())
			this->_msg += ": " + info;
	}
	
	const char* Exception::what() const noexcept
	{
		return this->_msg.c_str();
	}
}

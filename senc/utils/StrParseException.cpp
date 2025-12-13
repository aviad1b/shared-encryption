/*********************************************************************
 * \file   StrParseException.cpp
 * \brief  Implementation of StrParseException class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "StrParseException.hpp"

namespace senc::utils
{
	StrParseException::StrParseException(const std::string& msg)
		: Base(msg) { }

	StrParseException::StrParseException(std::string&& msg)
		: Base(std::move(msg)) { }

	StrParseException::StrParseException(const std::string& msg, const std::string& info)
		: Base(msg, info) { }

	StrParseException::StrParseException(std::string&& msg, const std::string& info)
		: Base(std::move(msg), info) { }
}

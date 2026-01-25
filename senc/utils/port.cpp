/*********************************************************************
 * \file   port.cpp
 * \brief  Implementation of transport layer port utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "port.hpp"

#include "StrParseException.hpp"
#include <limits>

namespace senc::utils
{
	Port parse_port(const std::string& str)
	{
		int port = 0;
		try { port = std::stoi(str); }
		catch (const std::exception&) { throw StrParseException("Bad port: " + str); }
		if (port < std::numeric_limits<Port>::min() || port > std::numeric_limits<Port>::max())
			throw StrParseException("Bad port: " + str);
		return static_cast<Port>(port);
	}
}

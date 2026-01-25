/*********************************************************************
 * \file   port.hpp
 * \brief  Header of transport layer port utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include <cstdint>
#include <string>

namespace senc::utils
{
	/**
	 * @typedef senc::utils::port
	 * @brief Represents a transport port number.
	 */
	using Port = std::uint16_t;

	/**
	 * @brief Parses port from string.
	 * @param str Port string to parse.
	 * @return Parsed port.
	 * @throw StrParseException If failed to parse (invalid port string).
	 */
	Port parse_port(const std::string& str);
}

/*********************************************************************
 * \file   ConsoleLogger.cpp
 * \brief  Implementation of ConsoleLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "ConsoleLogger.hpp"

namespace senc::server
{
	void ConsoleLogger::log_info(const std::string& msg)
	{
		this->_console.print("[info] " + msg);
	}
}

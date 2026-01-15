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
	ConsoleLogger::ConsoleLogger(InteractiveConsole& console)
		: _console(console) { }

	void ConsoleLogger::log_info(const std::string& msg)
	{
		this->_console.print("[info] " + msg);
	}
}

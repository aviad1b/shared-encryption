/*********************************************************************
 * \file   ConsoleLogger.cpp
 * \brief  Implementation of ConsoleLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "ConsoleLogger.hpp"

namespace senc::server::loggers
{
	ConsoleLogger::ConsoleLogger(io::InteractiveConsole& console)
		: _console(console) { }

	void ConsoleLogger::log_info(const std::string& msg)
	{
		const std::lock_guard<std::mutex> lock(_mtxConsole);
		this->_console.print("[info] " + msg);
	}

	void ConsoleLogger::log_warning(const std::string& msg)
	{
		const std::lock_guard<std::mutex> lock(_mtxConsole);
		this->_console.print("[warning] " + msg);
	}

	void ConsoleLogger::log_error(const std::string& msg)
	{
		const std::lock_guard<std::mutex> lock(_mtxConsole);
		this->_console.print("[error] " + msg);
	}
}

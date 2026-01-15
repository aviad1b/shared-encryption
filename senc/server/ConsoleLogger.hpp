/*********************************************************************
 * \file   ConsoleLogger.hpp
 * \brief  Header of ConsoleLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "InteractiveConsole.hpp"
#include "ILogger.hpp"

namespace senc::server
{
	class ConsoleLogger : public ILogger
	{
	public:
		using Self = ConsoleLogger;

		ConsoleLogger(InteractiveConsole& console);

		void log_info(const std::string& msg) override;

	private:
		InteractiveConsole& _console;
	};
}

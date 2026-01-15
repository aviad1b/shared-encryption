/*********************************************************************
 * \file   ConsoleLogger.hpp
 * \brief  Header of ConsoleLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "InteractiveConsole.hpp"
#include "Logger.hpp"

namespace senc::server
{
	/**
	 * @class senc::server::ConsoleLogger
	 * @brief Logger implementation based on interactive console.
	 */
	class ConsoleLogger : public Logger
	{
	public:
		using Self = ConsoleLogger;
		using Base = Logger;

		ConsoleLogger(InteractiveConsole& console);

		void log_info(const std::string& msg) override;

	private:
		InteractiveConsole& _console;
	};
}

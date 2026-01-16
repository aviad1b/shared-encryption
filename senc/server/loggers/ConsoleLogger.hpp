/*********************************************************************
 * \file   ConsoleLogger.hpp
 * \brief  Header of ConsoleLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../io/InteractiveConsole.hpp"
#include "ILogger.hpp"
#include <mutex>

namespace senc::server::loggers
{
	/**
	 * @class senc::server::loggers::ConsoleLogger
	 * @brief Logger implementation based on interactive console.
	 */
	class ConsoleLogger : public ILogger
	{
	public:
		using Self = ConsoleLogger;

		ConsoleLogger(io::InteractiveConsole& console);

		void log_info(const std::string& msg) override;
		
		void log_error(const std::string& msg) override;

	private:
		io::InteractiveConsole& _console;
		std::mutex _mtxConsole;
	};
}

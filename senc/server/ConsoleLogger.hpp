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
#include <mutex>

namespace senc::server
{
	/**
	 * @class senc::server::ConsoleLogger
	 * @brief Logger implementation based on interactive console.
	 */
	class ConsoleLogger : public ILogger
	{
	public:
		using Self = ConsoleLogger;

		ConsoleLogger(InteractiveConsole& console);

		void log_info(const std::string& msg) override;
		
		void log_error(const std::string& msg) override;

	private:
		InteractiveConsole& _console;
		std::mutex _mtxConsole;
	};
}

/*********************************************************************
 * \file   ILogger.hpp
 * \brief  Contains Logger interface.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/Socket.hpp"
#include <string>

namespace senc::server
{
	/**
	 * @interface senc::server::ILogger
	 * @brief Interface for server message logging.
	 */
	class ILogger
	{
	public:
		virtual ~ILogger() { }

		virtual void log_info(const std::string& msg) = 0;
	};
}

/*********************************************************************
 * \file   ILogger.hpp
 * \brief  Contains abstract Logger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include <string>

namespace senc::server
{
	/**
	 * @class senc::server::ILogger
	 * @brief Base class for server message logging.
	 */
	class Logger
	{
	public:
		virtual ~Logger() { }

		virtual void log_info(const std::string& msg) = 0;
	};
}

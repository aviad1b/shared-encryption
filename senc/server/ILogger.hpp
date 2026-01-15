/*********************************************************************
 * \file   ILogger.hpp
 * \brief  Contains ILogger interface.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include <string>

namespace senc::server
{
	/**
	 * @interface senc::server::ILogger
	 * @brief Interface of server message logging.
	 */
	class ILogger
	{
	public:
		virtual ~ILogger() { }

		virtual void log_info(const std::string& msg) = 0;
	};
}

/*********************************************************************
 * \file   ILogger.hpp
 * \brief  Contains abstract Logger class.
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
	 * @class senc::server::ILogger
	 * @brief Base class for server message logging.
	 */
	class Logger
	{
	public:
		virtual ~Logger() { }

		virtual void log_info(const std::string& msg) = 0;

		void log_info(const utils::IPType auto& ip, utils::Port port, const std::string& msg)
		{
			log_info("Client " + ip.as_str() + ":" + std::to_string(port) + ": " + msg);
		}

		void log_info(const utils::IPType auto& ip, utils::Port port, const std::string& username, const std::string& msg)
		{
			log_info("Client " + ip.as_str() + ":" + std::to_string(port) + " (\"" + username + "\"): " + msg);
		}
	};
}

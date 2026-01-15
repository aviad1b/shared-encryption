/*********************************************************************
 * \file   ConnectedLogger.hpp
 * \brief  Contains ConnectedLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/Socket.hpp"
#include "Logger.hpp"

namespace senc::server
{
	/**
	 * @class senc::server::ConnectedLogger
	 * @brief Logger wrapper used for logging info about a connected client.
	 */
	template <utils::IPType IP>
	class ConnectedLogger : public Logger
	{
	public:
		using Self = ConnectedLogger;
		using Base = Logger;

		/**
		 * @brief Constructs a logger instance for a connected client.
		 * @param base Base logger to use for logging.
		 * @param ip Connected client's IP address.
		 * @param port Connected client's port number.
		 * @param username Connected client's username.
		 */
		ConnectedLogger(Logger& base, const IP& ip, utils::Port port, const std::string& username)
			: _base(base), _ip(ip), _port(port), _username(username) { }

		void log_info(const std::string& msg) override
		{
			_base.log_info(
				"Client " + _ip.as_str() + ":" + std::to_string(_port) + " (\"" + _username + "\"): " + msg
			);
		}

	private:
		Logger& _base;
		const IP& _ip;
		utils::Port _port;
	};
}

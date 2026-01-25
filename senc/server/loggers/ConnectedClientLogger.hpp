/*********************************************************************
 * \file   ConnectedClientLogger.hpp
 * \brief  Contains ConnectedClientLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../../utils/ip.hpp"
#include "ILogger.hpp"

namespace senc::server::loggers
{
	/**
	 * @class senc::server::loggers::ConnectedLogger
	 * @brief Logger wrapper used for logging info about a connected client.
	 */
	template <utils::IPType IP>
	class ConnectedClientLogger : public ILogger
	{
	public:
		using Self = ConnectedClientLogger;

		/**
		 * @brief Constructs a logger instance for a connected client.
		 * @param base Base logger to use for logging.
		 * @param ip Connected client's IP address.
		 * @param port Connected client's port number.
		 * @param username Connected client's username.
		 */
		ConnectedClientLogger(ILogger& base, const IP& ip, utils::Port port, const std::string& username)
			: _base(base), _ip(ip), _port(port), _username(username) { }

		void log_info(const std::string& msg) override
		{
			_base.log_info(
				"Client " + _ip.as_str() + ":" + std::to_string(_port) + " (\"" + _username + "\"): " + msg
			);
		}

		void log_warning(const std::string& msg) override
		{
			_base.log_error(
				"Client " + _ip.as_str() + ":" + std::to_string(_port) + " (\"" + _username + "\"): " + msg
			);
		}

		void log_error(const std::string& msg) override
		{
			_base.log_error(
				"Client " + _ip.as_str() + ":" + std::to_string(_port) + " (\"" + _username + "\"): " + msg
			);
		}

	private:
		ILogger& _base;
		const IP& _ip;
		utils::Port _port;
		const std::string _username;
	};
}

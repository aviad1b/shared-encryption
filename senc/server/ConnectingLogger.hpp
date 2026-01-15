/*********************************************************************
 * \file   ConnectingLogger.hpp
 * \brief  Contains ConnectingLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/Socket.hpp"
#include "ILogger.hpp"

namespace senc::server
{
	/**
	 * @class senc::server::ConnectingLogger
	 * @brief Logger wrapper used for logging info about a connecting client.
	 */
	template <utils::IPType IP>
	class ConnectingLogger : public ILogger
	{
	public:
		using Self = ConnectingLogger;

		/**
		 * @brief Constructs a logger instance for a connecting client.
		 * @param base Base logger to use for logging.
		 * @param ip Connecting client's IP address.
		 * @param port Connecting client's port number.
		 */
		ConnectingLogger(ILogger& base, const IP& ip, utils::Port port)
			: _base(base), _ip(ip), _port(port) { }

		void log_info(const std::string& msg) override
		{
			_base.log_info("Client " + _ip.as_str() + ":" + std::to_string(_port) + ": " + msg);
		}

	private:
		ILogger& _base;
		const IP& _ip;
		utils::Port _port;
	};
}

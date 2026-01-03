/*********************************************************************
 * \file   IServer.hpp
 * \brief  Contains `IServer` interface.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/Socket.hpp"

namespace senc::server
{
	/**
	 * @interface senc::server::IServer
	 * @brief Defines public interface of `Server` class.
	 */
	class IServer
	{
	public:
		/**
		 * @brief Gets server listen port.
		 */
		virtual utils::Port port() const = 0;

		/**
		 * @brief Starts the server (without waiting).
		 */
		virtual void start() = 0;

		/**
		 * @brief Stops server sun.
		 */
		virtual void stop() = 0;

		/**
		 * @brief Waits for server to stop running.
		 */
		virtual void wait() = 0;
	};
}

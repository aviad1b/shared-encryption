/*********************************************************************
 * \file   IServer.hpp
 * \brief  Contains `IServer` interface.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

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

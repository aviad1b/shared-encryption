/*********************************************************************
 * \file   Server.hpp
 * \brief  Header of Server class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../common/PacketReceiver.hpp"
#include "../common/PacketSender.hpp"
#include "../utils/Socket.hpp"
#include "IServerStorage.hpp"
#include <condition_variable>
#include <atomic>
#include <mutex>

namespace senc::server
{
	/**
	 * @brief Server class, manages server logic.
	 */
	class Server
	{
	public:
		using Self = Server;
		using Socket = utils::TcpSocket<utils::IPv4>;

		/**
		 * @brief Constructs a new server instance.
		 * @param storage Implementation of `IServerStorage`.
		 * @param receiver Implementation of `PacketReceiver`.
		 * @param sender Implementation of `PacketSender`.
		 * @note `storage`, `receiver` and `sender` are all assumed to be thread-safe.
		 */
		explicit Server(utils::Port listenPort,
						IServerStorage& storage,
						PacketReceiver& receiver,
						PacketSender& sender);

		/**
		 * @brief Starts the server (without waiting).
		 */
		void start();

		/**
		 * @brief Stops server sun.
		 */
		void stop();

		/**
		 * @brief Waits for server to stop running.
		 */
		void wait();

	private:
		utils::Port _listenPort;
		IServerStorage& storage;
		PacketReceiver& receiver;
		PacketSender& sender;
		std::atomic<bool> _isRunning;

		std::mutex _mtxWait;
		std::condition_variable _cvWait;

		/**
		 * @brief Handles a newly connected client, until it disconnects.
		 * @param sock Socket connected to client (moved).
		 */
		void handle_new_client(Socket sock);
	};
}

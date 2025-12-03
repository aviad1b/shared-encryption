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
#include "ServerException.hpp"
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
		Socket _listenSock;
		utils::Port _listenPort;
		IServerStorage& _storage;
		PacketReceiver& _receiver;
		PacketSender& _sender;
		std::atomic<bool> _isRunning;

		std::mutex _mtxWait;
		std::condition_variable _cvWait;

		/**
		 * @brief Accepts new clients in a loop.
		 */
		void accept_loop();

		/**
		 * @brief Handles a newly connected client, until it disconnects.
		 * @param sock Socket connected to client (moved).
		 */
		void handle_new_client(Socket sock);

		/**
		 * @brief Handles new client connection with login request.
		 * @param sock Socket connected to client.
		 * @param login Login request sent by client.
		 */
		void client_connect(Socket& sock, const pkt::LoginRequest& login);
		
		/**
		 * @brief Handles new client connection with logout request.
		 * @param sock Socket connected to client.
		 * @param logout Logout request sent by client.
		 * @return `true` if finished handling client,
		 *		   otherwise `false` (if should go back to connection stage).
		 */
		void client_connect(Socket& sock, const pkt::LogoutRequest& logout);
	};
}

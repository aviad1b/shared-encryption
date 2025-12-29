/*********************************************************************
 * \file   Server.hpp
 * \brief  Header of Server class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "ClientHandlerFactory.hpp"
#include "ServerException.hpp"
#include <condition_variable>
#include <functional>
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
		 * @param listenPort Port for server to listen on.
		 * @param logInfo A function used to output server log information messages.
		 * @param schema Decryptions schema to use for decryptions.
		 * @param storage Implementation of `IServerStorage`.
		 * @param receiver Implementation of `PacketReceiver`.
		 * @param sender Implementation of `PacketSender`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage`, `receiver` and `sender` are all assumed to be thread-safe.
		 */
		explicit Server(utils::Port listenPort,
						std::optional<std::function<void(const std::string&)>> logInfo,
						Schema& schema,
						IServerStorage& storage,
						PacketReceiver& receiver,
						PacketSender& sender,
						UpdateManager& updateManager,
						DecryptionsManager& decryptionsManager);

		/**
		 * @brief Constructs a new server instance.
		 * @param listenPort Port for server to listen on.
		 * @param schema Decryptions schema to use for decryptions.
		 * @param storage Implementation of `IServerStorage`.
		 * @param receiver Implementation of `PacketReceiver`.
		 * @param sender Implementation of `PacketSender`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage`, `receiver` and `sender` are all assumed to be thread-safe.
		 */
		explicit Server(utils::Port listenPort,
						Schema& schema,
						IServerStorage& storage,
						PacketReceiver& receiver,
						PacketSender& sender,
						UpdateManager& updateManager,
						DecryptionsManager& decryptionsManager);

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
		std::optional<std::function<void(const std::string&)>> _logInfo;
		ClientHandlerFactory _clientHandlerFactory;
		std::atomic<bool> _isRunning;

		std::mutex _mtxWait;
		std::condition_variable _cvWait;

		enum class LogType { Info };

		/**
		 * @brief Outputs server log message.
		 * @param msg Message to output.
		 */
		void log(LogType logType, const std::string& msg);

		/**
		 * @brief Outputs server log message.
		 * @param ip Client's IP address.
		 * @param port Client's port.
		 * @param msg Message to output.
		 */
		void log(LogType logType, const utils::IPv4& ip, utils::Port port, const std::string& msg);
		
		/**
		 * @brief Outputs server log message.
		 * @param ip Client's IP address.
		 * @param port Client's port.
		 * @param username Client username.
		 * @param msg Message to output.
		 */
		void log(LogType logType, const utils::IPv4& ip, utils::Port port, const std::string& username, const std::string& msg);

		/**
		 * @brief Accepts new clients in a loop.
		 */
		void accept_loop();

		/**
		 * @brief Handles a newly connected client, until it disconnects.
		 * @param sock Socket connected to client (moved).
		 * @param ip IP address by which client connected.
		 * @param port Port by which client connected.
		 */
		void handle_new_client(Socket sock, utils::IPv4 ip, utils::Port port);

		/**
		 * @brief Handles client requests in a loop.
		 * @param sock Socket connected to client.
		 * @param username Client's connected username.
		 */
		void client_loop(Socket& sock, const std::string& username);
	};
}

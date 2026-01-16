/*********************************************************************
 * \file   Server.hpp
 * \brief  Header of Server class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../common/PacketHandlerFactory.hpp"
#include "handlers/ClientHandlerFactory.hpp"
#include "loggers/DummyLogger.hpp"
#include "loggers/ILogger.hpp"
#include "ServerException.hpp"
#include "IServer.hpp"
#include <condition_variable>
#include <functional>
#include <atomic>
#include <mutex>

namespace senc::server
{
	/**
	 * @class senc::server::Server
	 * @brief Server class, manages server logic.
	 * @tparam IP IP type used for communication.
	 */
	template <utils::IPType IP>
	class Server : public IServer
	{
	public:
		using Self = Server;
		using Socket = utils::TcpSocket<IP>;

		/**
		 * @brief Constructs a new server instance.
		 * @param listenPort Port for server to listen on.
		 * @param logger Implementation of `ILogger` for logging server messages.
		 * @param schema Decryptions schema to use for decryptions.
		 * @param storage Implementation of `IServerStorage`.
		 * @param packetHandlerFactory Factory constructing implementation of `PacketHandler`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage` and `packetHandler` are assumed to be thread-safe.
		 */
		explicit Server(utils::Port listenPort,
						loggers::ILogger& logger,
						Schema& schema,
						storage::IServerStorage& storage,
						PacketHandlerFactory& packetHandlerFactory,
						managers::UpdateManager& updateManager,
						managers::DecryptionsManager& decryptionsManager);

		/**
		 * @brief Constructs a new server instance.
		 * @param listenPort Port for server to listen on.
		 * @param schema Decryptions schema to use for decryptions.
		 * @param storage Implementation of `IServerStorage`.
		 * @param packetHandlerFactory Factory constructing implementation of `PacketHandler`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage` and `packetHandler` are assumed to be thread-safe.
		 */
		explicit Server(utils::Port listenPort,
						Schema& schema,
						storage::IServerStorage& storage,
						PacketHandlerFactory& packetHandlerFactory,
						managers::UpdateManager& updateManager,
						managers::DecryptionsManager& decryptionsManager);

		utils::Port port() const override;

		void start() override;

		void stop() override;

		void wait() override;

	private:
		static loggers::DummyLogger _dummyLogger;

		Socket _listenSock;
		utils::Port _listenPort;
		loggers::ILogger& _logger;
		PacketHandlerFactory _packetHandlerFactory;
		handlers::ClientHandlerFactory _clientHandlerFactory;
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
		 * @param ip IP address by which client connected.
		 * @param port Port by which client connected.
		 */
		void handle_new_client(Socket sock, IP ip, utils::Port port);

		/**
		 * @brief Handles client connection request(s).
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param ip Client's IP address.
		 * @param port Client's port number.
		 * @return A flag indicating if client successfully connected or not,
		 *		   and client's username (if successfully connected).
		 */
		std::pair<bool, std::string> connect_client(PacketHandler& packetHandler,
													const IP& ip,
													utils::Port port);

		/**
		 * @brief Handles client requests in a loop.
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param ip Client's IP address.
		 * @param port Client's port number.
		 * @param username Client's connected username.
		 */
		void client_loop(PacketHandler& packetHandler,
						 const IP& ip,
						 utils::Port port,
						 const std::string& username);
	};
}

#include "Server_impl.hpp"

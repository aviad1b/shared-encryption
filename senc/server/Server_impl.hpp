/*********************************************************************
 * \file   Server_impl.hpp
 * \brief  Implementation of Server class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "Server.hpp"

#include <optional>
#include "ConnectingLogger.hpp"
#include "ConnectedLogger.hpp"

namespace senc::server
{
	template <utils::IPType IP>
	DummyLogger Server<IP>::_dummyLogger;

	template <utils::IPType IP>
	inline Server<IP>::Server(utils::Port listenPort,
							  Logger& logger,
							  Schema& schema,
							  IServerStorage& storage,
							  PacketHandlerFactory& packetHandlerFactory,
							  UpdateManager& updateManager,
							  DecryptionsManager& decryptionsManager)
		: _listenPort(listenPort), _logger(logger), _packetHandlerFactory(packetHandlerFactory),
		  _clientHandlerFactory(schema, storage, updateManager, decryptionsManager)
	{
		_listenSock.bind(_listenPort);
	}

	template <utils::IPType IP>
	inline Server<IP>::Server(utils::Port listenPort,
							  Schema& schema,
							  IServerStorage& storage,
							  PacketHandlerFactory& packetHandlerFactory,
							  UpdateManager& updateManager,
							  DecryptionsManager& decryptionsManager)
		: Self(listenPort, _dummyLogger, schema, storage,
			   packetHandlerFactory, updateManager, decryptionsManager) { }

	template <utils::IPType IP>
	inline utils::Port Server<IP>::port() const
	{
		return this->_listenPort;
	}

	template <utils::IPType IP>
	inline void Server<IP>::start()
	{
		if (_isRunning.exchange(true))
			throw ServerException("Server is already running");
		
		_listenSock.listen();

		std::thread acceptThread(&Self::accept_loop, this);
		acceptThread.detach();
	}

	template <utils::IPType IP>
	inline void Server<IP>::stop()
	{
		if (!_isRunning.exchange(false))
			throw ServerException("Server is not running");

		_listenSock.close(); // forces stop of any hanging accepts

		_cvWait.notify_all(); // notify all waiting threads that finished running
	}

	template <utils::IPType IP>
	inline void Server<IP>::wait()
	{
		// use condition variable to wait untill !_isRunning
		std::unique_lock<std::mutex> lock(_mtxWait);
		_cvWait.wait(lock, [this]() { return !_isRunning; });
	}

	template <utils::IPType IP>
	inline void Server<IP>::accept_loop()
	{
		while (_isRunning)
		{
			std::optional<std::pair<Socket, std::tuple<IP, utils::Port>>> acceptRet;
			try { acceptRet = _listenSock.accept(); }
			catch (const utils::SocketException&) { continue; }
			// silently ignores failed accepts - might be due to server stop

			auto& [sock, addr] = *acceptRet;
			const auto& [ip, port] = addr;

			std::thread handleClientThread(
				&Self::handle_new_client, this,
				std::move(sock), std::move(ip), port
			);
			handleClientThread.detach();
		}
	}

	template <utils::IPType IP>
	inline void Server<IP>::handle_new_client(Socket sock, IP ip, utils::Port port)
	{
		auto packetHandler = _packetHandlerFactory.new_server_packet_handler(sock);
		bool connected = false;
		std::string username;

		{
			ConnectingLogger<IP> logger(_logger, ip, port);
			logger.log_info("Connected.");
			auto clientHandler = _clientHandlerFactory.make_connecting_client_handler(
				*packetHandler,
				logger
			);

			try { std::tie(connected, username) = clientHandler.connect_client(); }
			catch (const utils::SocketException& e)
			{
				logger.log_info(std::string("Lost connection: ") + e.what() + ".");
			}

			if (connected)
				logger.log_info("Logged in as \"" + username + "\".");
			else
				logger.log_info("Disconnected.");
		}

		if (connected)
			client_loop(*packetHandler, ip, port, username);
	}

	template <utils::IPType IP>
	inline void Server<IP>::client_loop(PacketHandler& packetHandler,
										const IP& ip,
										utils::Port port,
										const std::string& username)
	{
		ConnectedLogger<IP> logger(_logger, ip, port, username);
		auto handler = _clientHandlerFactory.make_connected_client_handler(
			packetHandler,
			logger,
			username
		);
		try { handler.loop(); }
		catch (const utils::SocketException& e)
		{
			logger.log_info(std::string("Lost connection: ") + e.what());
		}
		logger.log_info("Disconnected.");
	}
}

/*********************************************************************
 * \file   Server_impl.hpp
 * \brief  Implementation of Server class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "Server.hpp"

#include <optional>
#include "ConnectingClientHandler.hpp"
#include "ConnectedClientHandler.hpp"

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
	inline void Server<IP>::log(LogType logType, const std::string& msg)
	{
		(void)logType;
		_logger.log_info(msg);
	}

	template <utils::IPType IP>
	inline void Server<IP>::log(LogType logType, const IP& ip, utils::Port port, const std::string& msg)
	{
		(void)logType;
		_logger.log_info(ip, port, msg);
	}

	template <utils::IPType IP>
	inline void Server<IP>::log(LogType logType, const IP& ip, utils::Port port, const std::string& username, const std::string& msg)
	{
		(void)logType;
		_logger.log_info(ip, port, username, msg);
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

			log(LogType::Info, ip, port, "connected");

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
		auto clientHandler = _clientHandlerFactory.make_connecting_client_handler(*packetHandler);
		bool connected = false;
		std::string username;

		try { std::tie(connected, username) = clientHandler.connect_client(); }
		catch (const utils::SocketException& e)
		{
			log(LogType::Info, ip, port, std::string("lost connection: ") + e.what() + ".");
		}

		if (!connected)
			log(LogType::Info, ip, port, "disconnected.");
		else
		{
			log(LogType::Info, ip, port, "logged in as \"" + username + "\".");

			try { client_loop(*packetHandler, username); }
			catch (const utils::SocketException& e)
			{
				log(LogType::Info, ip, port, username, std::string("lost connection: ") + e.what());
			}

			log(LogType::Info, ip, port, username, "disconnected.");
		}
	}

	template <utils::IPType IP>
	inline void Server<IP>::client_loop(PacketHandler& packetHandler, const std::string& username)
	{
		auto handler = _clientHandlerFactory.make_connected_client_handler(packetHandler, username);
		handler.loop();
	}
}

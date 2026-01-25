/*********************************************************************
 * \file   Server_impl.hpp
 * \brief  Implementation of Server class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "Server.hpp"

#include <optional>
#include "loggers/ConnectingClientLogger.hpp"
#include "loggers/ConnectedClientLogger.hpp"
#include "../utils/AtScopeExit.hpp"

namespace senc::server
{
	template <utils::IPType IP>
	loggers::DummyLogger Server<IP>::_dummyLogger;

	template <utils::IPType IP>
	inline Server<IP>::Server(utils::Port listenPort,
							  loggers::ILogger& logger,
							  Schema& schema,
							  storage::IServerStorage& storage,
							  PacketHandlerFactory& packetHandlerFactory,
							  managers::UpdateManager& updateManager,
							  managers::DecryptionsManager& decryptionsManager)
		: _listenPort(listenPort), _logger(logger), _packetHandlerFactory(packetHandlerFactory),
		  _clientHandlerFactory(schema, storage, updateManager, decryptionsManager)
	{
		_listenSock.bind(_listenPort);
	}

	template <utils::IPType IP>
	inline Server<IP>::Server(utils::Port listenPort,
							  Schema& schema,
							  storage::IServerStorage& storage,
							  PacketHandlerFactory& packetHandlerFactory,
							  managers::UpdateManager& updateManager,
							  managers::DecryptionsManager& decryptionsManager)
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

		_acceptThread.emplace(&Self::accept_loop, this);
		_cleanupThread.emplace(&Self::cleanup_loop, this);
	}

	template <utils::IPType IP>
	inline void Server<IP>::stop()
	{
		if (!_isRunning.exchange(false))
			throw ServerException("Server is not running");

		_listenSock.close(); // forces stop of any hanging accepts

		_cvFinishedConns.notify_all(); // final cleanups

		// force close all client sockets and wait for all client threads to exit gracefully
		{
			// NOTE: locks are aquired in this order for consistency with accept
			const std::lock_guard<std::mutex> lock(_mtxClients);
			for (auto& p : _clientSocks)
				p.second.get().close();
			_clientSocks.clear();
			_clientThreads.clear(); // calls jthread dtors
			_finishedConns.clear();
		}

		// wait for accept loop and cleanup loop threads to finish gracefully
		_acceptThread.reset();
		_cleanupThread.reset();

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
	inline void Server<IP>::cleanup_loop()
	{
		while (_isRunning)
		{
			std::unique_lock<std::mutex> lock(_mtxClients);
			_cvFinishedConns.wait(lock, [this]()
			{
				return (!_isRunning || !this->_finishedConns.empty());
			});

			// if server stopped mid-way, return
			if (!_isRunning)
				return;

			// for each finished connection, remove its matching thread
			for (const auto& connID : _finishedConns)
			{
				const auto it = _clientThreads.find(connID);
				if (it != _clientThreads.end())
					_clientThreads.erase(it);
			}

			_finishedConns.clear();
		}
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

			const std::lock_guard<std::mutex> lock(_mtxClients);
			auto connID = utils::UUID::generate_not_in(_clientThreads);
			_clientThreads.emplace(
				connID, std::jthread(
					&Self::handle_new_client, this,
					std::move(connID), std::move(sock), std::move(ip), port
				)
			);
		}
	}

	template <utils::IPType IP>
	inline void Server<IP>::handle_new_client(utils::UUID connID, Socket sock, IP ip, utils::Port port)
	{
		// register client socket
		{
			const std::lock_guard<std::mutex> lock(_mtxClients);
			_clientSocks.emplace(connID, sock);
		}

		// at scope exit, clean up socket and mark connection as finished
		utils::AtScopeExit cleanup([this, &connID]()
		{
			const std::lock_guard<std::mutex> lock(_mtxClients);
			this->_clientSocks.erase(connID);
			this->_finishedConns.insert(connID);
			_cvFinishedConns.notify_one();
		});

		// if server stopped mid-way, return
		if (!_isRunning)
			return;

		auto packetHandler = _packetHandlerFactory.new_server_packet_handler(sock);
		
		const auto [connected, username] = connect_client(*packetHandler, ip, port);

		if (connected)
			client_loop(*packetHandler, ip, port, username);
	}

	template <utils::IPType IP>
	inline std::pair<bool, std::string> Server<IP>::connect_client(PacketHandler& packetHandler, const IP& ip, utils::Port port)
	{
		using Status = handlers::ConnectingClientHandler::Status;

		loggers::ConnectingClientLogger<IP> logger(_logger, ip, port);
		logger.log_info("Connected.");
		auto clientHandler = _clientHandlerFactory.make_connecting_client_handler(
			packetHandler
		);

		Status status = Status::Error;
		std::string username;
		while (Status::Error == status && _isRunning)
		{
			try { std::tie(status, username) = clientHandler.iteration(); }
			catch (const utils::SocketException& e)
			{
				// might happened because server stopped, in that case, stop here
				if (!_isRunning)
					return { false, "" };

				logger.log_info(std::string("Lost connection: ") + e.what() + ".");
			}
			catch (const std::exception& e)
			{
				_logger.log_error(std::string("Failed to handle request: ") + e.what() + ".");
			}
		};

		// if server stopped mid-way, stop here
		if (!_isRunning)
			return { false, "" };

		const bool connected = (Status::Connected == status);
		if (connected)
			logger.log_info("Logged in as \"" + username + "\".");
		else
			logger.log_info("Disconnected.");

		return std::make_pair(connected, username);
	}

	template <utils::IPType IP>
	inline void Server<IP>::client_loop(PacketHandler& packetHandler,
										const IP& ip,
										utils::Port port,
										const std::string& username)
	{
		using Status = handlers::ConnectedClientHandler::Status;
		loggers::ConnectedClientLogger<IP> logger(_logger, ip, port, username);
		auto handler = _clientHandlerFactory.make_connected_client_handler(
			packetHandler,
			username
		);

		Status status = Status::Connected;
		while (Status::Connected == status && _isRunning)
		{
			try { status = handler.iteration(); }
			catch (const utils::SocketException& e)
			{
				// might happened because server stopped, in that case, stop here
				if (!_isRunning)
					return;

				logger.log_info(std::string("Lost connection: ") + e.what());
			}
			catch (const std::exception& e)
			{
				logger.log_error(std::string("Failed to handle request: ") + e.what() + ".");
			}
		}

		// if server stopped mid-way, stop here
		if (!_isRunning)
			return;

		logger.log_info("Disconnected.");
	}
}

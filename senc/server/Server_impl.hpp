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

		std::thread acceptThread(&Self::accept_loop, this);
		acceptThread.detach();

		std::thread cleanupThread(&Self::cleanup_loop, this);
		cleanupThread.detach();
	}

	template <utils::IPType IP>
	inline void Server<IP>::stop()
	{
		if (!_isRunning.exchange(false))
			throw ServerException("Server is not running");

		_listenSock.close(); // forces stop of any hanging accepts

		finish_all_conns(); // close all open connections

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
	inline void Server<IP>::reg_finished_conn(utils::UUID&& connID)
	{
		const std::lock_guard<std::mutex> lock(_mtxFinishedConns);
		_finishedConns.emplace_back(std::move(connID));
		_cvFinishedConns.notify_one();
	}

	template <utils::IPType IP>
	inline void Server<IP>::finish_all_conns()
	{
		const std::lock_guard<std::mutex> lock(_mtxConns);
		// first, close all sockets; then, join all threads
		for (auto& p : _conns)
			p.second.first.close();
		for (auto& p : _conns)
			if (p.second.second.joinable())
				p.second.second.join();
	}

	template <utils::IPType IP>
	inline void Server<IP>::cleanup_loop()
	{
		while (_isRunning)
		{
			std::unique_lock lock(_mtxFinishedConns);
			_cvFinishedConns.wait(lock, [this]() { return !this->_finishedConns.empty(); });

			for (const auto& connID : _finishedConns)
			{
				const auto it = _conns.find(connID);
				if (it == _conns.end())
					continue;
				// first, close socket; then, join thread
				it->second.first.close();
				if (it->second.second.joinable())
					it->second.second.join();
			}
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

			// register connection
			const std::lock_guard<std::mutex> lock(_mtxConns);
			auto connID = utils::UUID::generate_not_in(_conns);
			std::thread handleClientThread(
				&Self::handle_new_client, this,
				connID, std::move(ip), port
			);
			_conns.insert(std::make_pair(
				connID,
				std::make_pair(std::move(sock), std::move(handleClientThread))
			));
		}
	}

	template <utils::IPType IP>
	inline void Server<IP>::handle_new_client(utils::UUID connID, IP ip, utils::Port port)
	{
		// get socket from connections map and make a packet handler from it
		std::unique_ptr<PacketHandler> packetHandler;
		{
			const std::lock_guard<std::mutex> lock(_mtxConns);
			packetHandler = _packetHandlerFactory.new_server_packet_handler(_conns.at(connID).first);
		}
		
		const auto [connected, username] = connect_client(*packetHandler, ip, port);

		if (connected)
			client_loop(*packetHandler, ip, port, username);

		// register finished connection
		reg_finished_conn(std::move(connID));
	}

	template <utils::IPType IP>
	inline std::pair<bool, std::string> Server<IP>::connect_client(PacketHandler& packetHandler, const IP& ip, utils::Port port)
	{
		using Status = handlers::ConnectingClientHandler::Status;

		loggers::ConnectingClientLogger<IP> logger(_logger, ip, port);
		logger.log_info("Connected.");
		auto clientHandler = _clientHandlerFactory.make_connecting_client_handler(
			packetHandler,
			logger
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
			logger,
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
		logger.log_info("Disconnected.");
	}
}

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
	inline Server::Server(utils::Port listenPort,
						  std::optional<std::function<void(const std::string&)>> logInfo,
						  Schema& schema,
						  IServerStorage& storage,
						  PacketReceiver& receiver,
						  PacketSender& sender,
						  UpdateManager& updateManager,
						  DecryptionsManager& decryptionsManager)
		: _listenPort(listenPort), _logInfo(logInfo),
		  _clientHandlerFactory(schema, storage, receiver, sender, updateManager, decryptionsManager)
	{
		_listenSock.bind(_listenPort);
	}

	inline Server::Server(utils::Port listenPort,
						  Schema& schema,
						  IServerStorage& storage,
						  PacketReceiver& receiver,
						  PacketSender& sender,
						  UpdateManager& updateManager,
						  DecryptionsManager& decryptionsManager)
		: Self(listenPort, std::nullopt, schema, storage,
			   receiver, sender, updateManager, decryptionsManager) { }

	inline void Server::start()
	{
		if (_isRunning.exchange(true))
			throw ServerException("Server is already running");
		
		_listenSock.listen();

		std::thread acceptThread(&Self::accept_loop, this);
		acceptThread.detach();
	}

	inline void Server::stop()
	{
		if (!_isRunning.exchange(false))
			throw ServerException("Server is not running");

		_listenSock.close(); // forces stop of any hanging accepts

		_cvWait.notify_all(); // notify all waiting threads that finished running
	}

	inline void Server::wait()
	{
		// use condition variable to wait untill !_isRunning
		std::unique_lock<std::mutex> lock(_mtxWait);
		_cvWait.wait(lock, [this]() { return !_isRunning; });
	}

	inline void Server::log(LogType logType, const std::string& msg)
	{
		(void)logType;
		if (_logInfo.has_value())
			(*_logInfo)(msg);
	}

	inline void Server::log(LogType logType, const utils::IPv4& ip, utils::Port port, const std::string& msg)
	{
		log(logType, "Client " + ip.as_str() + ":" + std::to_string(port) + " " + msg);
	}

	inline void Server::log(LogType logType, const utils::IPv4& ip, utils::Port port, const std::string& username, const std::string& msg)
	{
		log(logType, ip, port, "(\"" + username + "\") " + msg);
	}

	inline void Server::accept_loop()
	{
		while (_isRunning)
		{
			std::optional<std::pair<Socket, std::tuple<utils::IPv4, utils::Port>>> acceptRet;
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

	inline void Server::handle_new_client(Socket sock, utils::IPv4 ip, utils::Port port)
	{
		auto handler = _clientHandlerFactory.make_connecting_client_handler(sock);
		bool connected = false;
		std::string username;

		try { std::tie(connected, username) = handler.connect_client(); }
		catch (const utils::SocketException& e)
		{
			log(LogType::Info, ip, port, std::string("lost connection: ") + e.what() + ".");
		}

		if (!connected)
			log(LogType::Info, ip, port, "disconnected.");
		else
		{
			log(LogType::Info, ip, port, "logged in as \"" + username + "\".");

			try { client_loop(sock, username); }
			catch (const utils::SocketException& e)
			{
				log(LogType::Info, ip, port, username, std::string("lost connection: ") + e.what());
			}

			log(LogType::Info, ip, port, username, "disconnected.");
		}
	}

	inline void Server::client_loop(Socket& sock, const std::string& username)
	{
		auto handler = _clientHandlerFactory.make_connected_client_handler(sock, username);
		handler.loop();
	}
}

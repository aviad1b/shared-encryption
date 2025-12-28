/*********************************************************************
 * \file   Server.cpp
 * \brief  Implementation of Server class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "Server.hpp"

#include <optional>
#include "ConnectingClientHandler.hpp"
#include "ConnectedClientHandler.hpp"

namespace senc::server
{
	Server::Server(utils::Port listenPort,
				   std::optional<std::function<void(const std::string&)>> log,
				   Schema& schema,
				   IServerStorage& storage,
				   PacketReceiver& receiver,
				   PacketSender& sender,
				   UpdateManager& updateManager,
				   DecryptionsManager& decryptionsManager)
		: _listenPort(listenPort), _log(log),
		  _clientHandlerFactory(schema, storage, receiver, sender, updateManager, decryptionsManager)
	{
		_listenSock.bind(_listenPort);
	}

	Server::Server(utils::Port listenPort,
				   Schema& schema,
				   IServerStorage& storage,
				   PacketReceiver& receiver,
				   PacketSender& sender,
				   UpdateManager& updateManager,
				   DecryptionsManager& decryptionsManager)
		: Self(listenPort, std::nullopt, schema, storage,
			   receiver, sender, updateManager, decryptionsManager) { }

	void Server::start()
	{
		if (_isRunning.exchange(true))
			throw ServerException("Server is already running");
		
		_listenSock.listen();

		std::thread acceptThread(&Self::accept_loop, this);
		acceptThread.detach();
	}

	void Server::stop()
	{
		if (!_isRunning.exchange(false))
			throw ServerException("Server is not running");

		_listenSock.close(); // forces stop of any hanging accepts

		_cvWait.notify_all(); // notify all waiting threads that finished running
	}

	void Server::wait()
	{
		// use condition variable to wait untill !_isRunning
		std::unique_lock<std::mutex> lock(_mtxWait);
		_cvWait.wait(lock, [this]() { return !_isRunning; });
	}

	void Server::log(const std::string& msg)
	{
		if (_log.has_value())
			(*_log)("[info] " + msg);
	}

	void Server::log(const utils::IPv4& ip, utils::Port port, const std::string& msg)
	{
		log("Client " + ip.as_str() + ":" + std::to_string(port) + " " + msg);
	}

	void Server::log(const utils::IPv4& ip, utils::Port port, const std::string& username, const std::string& msg)
	{
		log(ip, port, "(\"" + username + "\") " + msg);
	}

	void Server::accept_loop()
	{
		while (_isRunning)
		{
			std::optional<std::pair<Socket, std::tuple<utils::IPv4, utils::Port>>> acceptRet;
			try { acceptRet = _listenSock.accept(); }
			catch (const utils::SocketException&) { continue; }
			// silently ignores failed accepts - might be due to server stop

			auto& [sock, addr] = *acceptRet;
			const auto& [ip, port] = addr;

			log(ip, port, "connected");

			std::thread handleClientThread(
				&Self::handle_new_client, this,
				std::move(sock), std::move(ip), port
			);
			handleClientThread.detach();
		}
	}

	void Server::handle_new_client(Socket sock, utils::IPv4 ip, utils::Port port)
	{
		auto handler = _clientHandlerFactory.make_connecting_client_handler(sock);
		bool connected = false;
		std::string username;

		try { std::tie(connected, username) = handler.connect_client(); }
		catch (const utils::SocketException& e)
		{
			log(ip, port, std::string("lost connection: ") + e.what() + ".");
		}

		if (!connected)
			log(ip, port, "disconnected.");
		else
		{
			log(ip, port, "logged in as \"" + username + "\".");

			try { client_loop(sock, username); }
			catch (const utils::SocketException& e)
			{
				log(ip, port, username, std::string("lost connection: ") + e.what());
			}

			log(ip, port, username, "disconnected.");
		}
	}

	void Server::client_loop(Socket& sock, const std::string& username)
	{
		auto handler = _clientHandlerFactory.make_connected_client_handler(sock, username);
		handler.loop();
	}
}

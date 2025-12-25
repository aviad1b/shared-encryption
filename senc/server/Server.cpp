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
		if (_isRunning)
			throw ServerException("Server is already running");
		_isRunning = true;
		
		_listenSock.listen();

		std::thread acceptThread(&Self::accept_loop, this);
		acceptThread.detach();
	}

	void Server::stop()
	{
		if (!_isRunning)
			throw ServerException("Server is not running");
		_isRunning = false;

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
			(*_log)(msg);
	}

	void Server::accept_loop()
	{
		while (_isRunning)
		{
			std::optional<Socket> sock;
			try { sock = _listenSock.accept().first; }
			catch (const utils::SocketException&) { continue; }
			// silently ignores failed accepts - might be due to server stop

			std::thread handleClientThread(&Self::handle_new_client, this, std::move(*sock));
			handleClientThread.detach();
		}
	}

	void Server::handle_new_client(Socket sock)
	{
		auto handler = _clientHandlerFactory.make_connecting_client_handler(sock);
		try
		{
			auto [connected, username] = handler.connect_client();
			if (connected)
				client_loop(sock, username);
		}
		catch (const utils::SocketException&) { }
		// silently ignores failed receives - assumes client disconnected
	}

	void Server::client_loop(Socket& sock, const std::string& username)
	{
		auto handler = _clientHandlerFactory.make_connected_client_handler(sock, username);
		handler.loop();
	}
}

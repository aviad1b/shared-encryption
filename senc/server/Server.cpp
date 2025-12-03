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
				   IServerStorage& storage,
				   PacketReceiver& receiver,
				   PacketSender& sender)
		: _listenPort(listenPort), _storage(storage),
		  _receiver(receiver), _sender(sender)
	{
		_listenSock.bind(_listenPort);
	}

	void Server::start()
	{
		if (_isRunning)
			throw ServerException("Server is already running");
		
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

	void Server::accept_loop()
	{
		while (_isRunning)
		{
			std::optional<Socket> sock;
			try { sock = _listenSock.accept(); }
			catch (utils::SocketException&) { continue; }
			// silently ignores failed accepts - might be due to server stop

			std::thread handleClientThread(&Self::handle_new_client, this, std::move(*sock));
			handleClientThread.detach();
		}
	}

	void Server::handle_new_client(Socket sock)
	{
		ConnectingClientHandler handler(
			sock,
			_storage,
			_receiver,
			_sender
		);
		auto [connected, username] = handler.connect_client();
		if (connected)
			client_loop(sock, username);
	}

	void Server::client_loop(Socket& sock, const std::string& username)
	{
		ConnectedClientHandler handler(
			sock,
			username,
			_storage,
			_receiver,
			_sender
		);
		handler.loop();
	}
}

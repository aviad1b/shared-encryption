/*********************************************************************
 * \file   Server.cpp
 * \brief  Implementation of Server class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "Server.hpp"

#include <optional>

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
		// connection request: Should either be login or logout (to disconnect)
		auto connReq = _receiver.recv_request<pkt::LoginRequest, pkt::LogoutRequest>(sock);
		while (!connReq.has_value())
		{
			_sender.send_response(sock, pkt::ErrorResponse{ "Bad request" });
			connReq = _receiver.recv_request<pkt::LoginRequest, pkt::LogoutRequest>(sock);
		}

		// call fitting client_loop implementation based on connection request
		std::visit(
			[&sock, this](const auto& req) { client_loop(sock, req); },
			*connReq
		);
	}

	void Server::client_loop(Socket& sock, const pkt::LogoutRequest& logout)
	{
		(void)logout;
		_sender.send_response(sock, pkt::LogoutResponse{});
	}
}

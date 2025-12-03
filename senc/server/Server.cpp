/*********************************************************************
 * \file   Server.cpp
 * \brief  Implementation of Server class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "Server.hpp"

namespace senc::server
{
	Server::Server(utils::Port listenPort,
				   IServerStorage& storage,
				   PacketReceiver& receiver,
				   PacketSender& sender)
		: _listenPort(listenPort), _storage(storage),
		  _receiver(receiver), _sender(sender) { }

	void Server::start()
	{
		if (_isRunning)
			throw ServerException("Server is already running");
		
		Socket listenSock;
		listenSock.bind(_listenPort);
		listenSock.listen();

		std::thread acceptThread(&Self::accept_loop, this, std::ref(listenSock));
		acceptThread.detach();
	}

	void Server::stop()
	{
		if (!_isRunning)
			throw ServerException("Server is not running");
		_isRunning = false;
		_cvWait.notify_all(); // notify all waiting threads that finished running
	}

	void Server::wait()
	{
		// use condition variable to wait untill !_isRunning
		std::unique_lock<std::mutex> lock(_mtxWait);
		_cvWait.wait(lock, [this]() { return !_isRunning; });
	}
}

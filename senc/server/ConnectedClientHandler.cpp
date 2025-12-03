/*********************************************************************
 * \file   ConnectedClientHandler.cpp
 * \brief  Implementation of ConnectedClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ConnectedClientHandler.hpp"

namespace senc::server
{
	ConnectedClientHandler::ConnectedClientHandler(utils::Socket& sock,
												   const std::string& username,
												   IServerStorage& storage,
												   PacketReceiver& receiver,
												   PacketSender& sender)
		: _sock(sock), _username(username), _storage(storage),
		  _receiver(receiver), _sender(sender) { }

	void ConnectedClientHandler::loop()
	{
		// TODO: Implement
	}
}

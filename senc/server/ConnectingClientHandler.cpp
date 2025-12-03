/*********************************************************************
 * \file   ConnectingClientHandler.cpp
 * \brief  Implementation of ConnectingClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ConnectingClientHandler.hpp"

namespace senc::server
{
	ConnectingClientHandler::ConnectingClientHandler(utils::Socket& sock,
													 IServerStorage& storage,
													 PacketReceiver& receiver,
													 PacketSender& sender)
		: _sock(sock), _storage(storage), _receiver(receiver), _sender(sender) { }
}

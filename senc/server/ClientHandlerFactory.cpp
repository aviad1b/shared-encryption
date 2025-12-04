/*********************************************************************
 * \file   ClientHandlerFactory.cpp
 * \brief  Implementation of ClientHandlerFactory class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ClientHandlerFactory.hpp"

namespace senc::server
{
	ClientHandlerFactory::ClientHandlerFactory(Schema& schema,
											   IServerStorage& storage,
											   PacketReceiver& receiver,
											   PacketSender& sender,
											   UpdateManager& updateManager,
											   DecryptionsManager& decryptionsManager)
		: _schema(schema), _storage(storage), _receiver(receiver), _sender(sender),
		  _updateManager(updateManager), _decryptionsManager(decryptionsManager) { }

	ConnectingClientHandler ClientHandlerFactory::make_connecting_client_handler(utils::Socket& sock)
	{
		return ConnectingClientHandler(
			sock,
			_storage,
			_receiver,
			_sender
		);
	}

	ConnectedClientHandler ClientHandlerFactory::make_connected_client_handler(utils::Socket& sock, const std::string& username)
	{
		return ConnectedClientHandler(
			sock,
			username,
			_schema,
			_storage,
			_receiver,
			_sender,
			_updateManager,
			_decryptionsManager
		);
	}
}

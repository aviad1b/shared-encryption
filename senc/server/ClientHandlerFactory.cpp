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
											   UpdateManager& updateManager,
											   DecryptionsManager& decryptionsManager)
		: _schema(schema), _storage(storage), _updateManager(updateManager),
		  _decryptionsManager(decryptionsManager) { }

	ConnectingClientHandler ClientHandlerFactory::make_connecting_client_handler(PacketHandler& packetHandler)
	{
		return ConnectingClientHandler(
			packetHandler,
			_storage
		);
	}

	ConnectedClientHandler ClientHandlerFactory::make_connected_client_handler(PacketHandler& packetHandler, const std::string& username)
	{
		return ConnectedClientHandler(
			packetHandler,
			username,
			_schema,
			_storage,
			_updateManager,
			_decryptionsManager
		);
	}
}

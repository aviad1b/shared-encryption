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
	ClientHandlerFactory::ClientHandlerFactory(Logger& logger,
											   Schema& schema,
											   IServerStorage& storage,
											   UpdateManager& updateManager,
											   DecryptionsManager& decryptionsManager)
		: _logger(logger), _schema(schema), _storage(storage), _updateManager(updateManager),
		  _decryptionsManager(decryptionsManager) { }

	ConnectingClientHandler ClientHandlerFactory::make_connecting_client_handler(PacketHandler& packetHandler)
	{
		return ConnectingClientHandler(
			_logger,
			packetHandler,
			_storage
		);
	}

	ConnectedClientHandler ClientHandlerFactory::make_connected_client_handler(PacketHandler& packetHandler, const std::string& username)
	{
		return ConnectedClientHandler(
			_logger,
			packetHandler,
			username,
			_schema,
			_storage,
			_updateManager,
			_decryptionsManager
		);
	}
}

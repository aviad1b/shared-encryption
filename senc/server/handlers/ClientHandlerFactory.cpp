/*********************************************************************
 * \file   ClientHandlerFactory.cpp
 * \brief  Implementation of ClientHandlerFactory class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ClientHandlerFactory.hpp"

namespace senc::server::handlers
{
	ClientHandlerFactory::ClientHandlerFactory(Schema& schema,
											   IServerStorage& storage,
											   UpdateManager& updateManager,
											   DecryptionsManager& decryptionsManager)
		: _schema(schema), _storage(storage), _updateManager(updateManager),
		  _decryptionsManager(decryptionsManager) { }

	ConnectingClientHandler ClientHandlerFactory::make_connecting_client_handler(PacketHandler& packetHandler,
																				 loggers::ILogger& logger)
	{
		return ConnectingClientHandler(
			logger,
			packetHandler,
			_storage
		);
	}

	ConnectedClientHandler ClientHandlerFactory::make_connected_client_handler(PacketHandler& packetHandler,
																			   loggers::ILogger& logger,
																			   const std::string& username)
	{
		return ConnectedClientHandler(
			logger,
			packetHandler,
			username,
			_schema,
			_storage,
			_updateManager,
			_decryptionsManager
		);
	}
}

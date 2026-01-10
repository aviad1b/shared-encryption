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
											   PacketHandler& packetHandler,
											   UpdateManager& updateManager,
											   DecryptionsManager& decryptionsManager)
		: _schema(schema), _storage(storage), _packetHandler(packetHandler),
		  _updateManager(updateManager), _decryptionsManager(decryptionsManager) { }

	ConnectingClientHandler ClientHandlerFactory::make_connecting_client_handler(utils::Socket& sock)
	{
		return ConnectingClientHandler(
			sock,
			_storage,
			_packetHandler
		);
	}

	ConnectedClientHandler ClientHandlerFactory::make_connected_client_handler(utils::Socket& sock, const std::string& username)
	{
		return ConnectedClientHandler(
			sock,
			username,
			_schema,
			_storage,
			_packetHandler,
			_updateManager,
			_decryptionsManager
		);
	}
}

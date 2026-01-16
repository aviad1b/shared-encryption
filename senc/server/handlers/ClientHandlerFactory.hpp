/*********************************************************************
 * \file   ClientHandlerFactory.hpp
 * \brief  Header of ClientHandlerFactory class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../../common/PacketHandler.hpp"
#include "ConnectingClientHandler.hpp"
#include "ConnectedClientHandler.hpp"
#include "../DecryptionsManager.hpp"
#include "../../utils/Socket.hpp"
#include "../IServerStorage.hpp"
#include "../UpdateManager.hpp"
#include "../ILogger.hpp"

namespace senc::server::handlers
{
	/**
	 * @class senc::server::handlers::ClientHandlerFactory
	 * @brief Used for constructing client handlers.
	 */
	class ClientHandlerFactory
	{
	public:
		using Self = ClientHandlerFactory;

		/**
		 * @brief Constructs a new client handler factory.
		 * @param schema Decryptions schema to use for decryptions.
		 * @param storage Implementation of `IServerStorage`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage` and `packetHandler` are assumed to be thread-safe.
		 */
		explicit ClientHandlerFactory(Schema& schema,
									  IServerStorage& storage,
									  UpdateManager& updateManager,
									  DecryptionsManager& decryptionsManager);

		/**
		 * @brief Constructs a new handler for a connecting client.
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param logger Implementation of `ILogger` for logging server messages about client.
		 * @return Constructed handler.
		 */
		ConnectingClientHandler make_connecting_client_handler(PacketHandler& packetHandler,
															   ILogger& logger);

		/**
		 * @brief Constructs a new handler for a connected client.
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param logger Implementation of `ILogger` for logging server messages about client.
		 * @param username Connected client's username.
		 */
		ConnectedClientHandler make_connected_client_handler(PacketHandler& packetHandler,
															 ILogger& logger,
															 const std::string& username);

	private:
		Schema& _schema;
		IServerStorage& _storage;
		UpdateManager& _updateManager;
		DecryptionsManager& _decryptionsManager;
	};
}

/*********************************************************************
 * \file   ClientHandlerFactory.hpp
 * \brief  Header of ClientHandlerFactory class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../../common/PacketHandler.hpp"
#include "../managers/DecryptionsManager.hpp"
#include "../managers/UpdateManager.hpp"
#include "../storage/IServerStorage.hpp"
#include "ConnectingClientHandler.hpp"
#include "ConnectedClientHandler.hpp"

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
									  storage::IServerStorage& storage,
									  managers::UpdateManager& updateManager,
									  managers::DecryptionsManager& decryptionsManager);

		/**
		 * @brief Constructs a new handler for a connecting client.
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @return Constructed handler.
		 */
		ConnectingClientHandler make_connecting_client_handler(PacketHandler& packetHandler);

		/**
		 * @brief Constructs a new handler for a connected client.
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param username Connected client's username.
		 */
		ConnectedClientHandler make_connected_client_handler(PacketHandler& packetHandler,
															 const std::string& username);

	private:
		Schema& _schema;
		storage::IServerStorage& _storage;
		managers::UpdateManager& _updateManager;
		managers::DecryptionsManager& _decryptionsManager;
	};
}

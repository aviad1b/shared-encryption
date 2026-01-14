/*********************************************************************
 * \file   ClientHandlerFactory.hpp
 * \brief  Header of ClientHandlerFactory class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "ConnectingClientHandler.hpp"
#include "ConnectedClientHandler.hpp"
#include "../common/PacketHandler.hpp"
#include "../utils/Socket.hpp"
#include "DecryptionsManager.hpp"
#include "UpdateManager.hpp"
#include "IServerStorage.hpp"

namespace senc::server
{
	/**
	 * @class senc::server::ClientHandlerFactory
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
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage` and `packetHandler` are assumed to be thread-safe.
		 */
		explicit ClientHandlerFactory(Schema& schema,
									  IServerStorage& storage,
									  PacketHandler& handler,
									  UpdateManager& updateManager,
									  DecryptionsManager& decryptionsManager);

		/**
		 * @brief Constructs a new handler for a connecting client.
		 * @param sock Socket connected to client.
		 * @return Constructed handler.
		 */
		ConnectingClientHandler make_connecting_client_handler(utils::Socket& sock);

		/**
		 * @brief Constructs a new handler for a connected client.
		 * @param sock Socket connected to client.
		 * @param username Connected client's username.
		 */
		ConnectedClientHandler make_connected_client_handler(utils::Socket& sock, const std::string& username);

	private:
		Schema& _schema;
		IServerStorage& _storage;
		PacketHandler& _packetHandler;
		UpdateManager& _updateManager;
		DecryptionsManager& _decryptionsManager;
	};
}

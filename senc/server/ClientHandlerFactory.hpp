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
#include "../common/PacketReceiver.hpp"
#include "../common/PacketSender.hpp"
#include "../utils/Socket.hpp"
#include "IServerStorage.hpp"

namespace senc::server
{
	/**
	 * @brief Used for constructing client handlers.
	 */
	class ClientHandlerFactory
	{
	public:
		/**
		 * @brief Constructs a new client handler factory.
		 * @param storage Implementation of `IServerStorage`.
		 * @param receiver Implementation of `PacketReceiver`.
		 * @param sender Implementation of `PacketSender`.
		 * @note `storage`, `receiver` and `sender` are all assumed to be thread-safe.
		 */
		explicit ClientHandlerFactory(IServerStorage& storage,
									  PacketReceiver& receiver,
									  PacketSender& sender);

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
		IServerStorage& _storage;
		PacketReceiver& _receiver;
		PacketSender& _sender;
	};
}

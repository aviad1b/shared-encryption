/*********************************************************************
 * \file   ConnectingClientHandler.hpp
 * \brief  Header of ConnectingClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../common/PacketReceiver.hpp"
#include "../common/PacketSender.hpp"
#include "../utils/Socket.hpp"
#include "ServerException.hpp"
#include "IServerStorage.hpp"
#include <tuple>

namespace senc::server
{
	/**
	 * @brief Handles requests of connecting client (not yet signed in).
	 */
	class ConnectingClientHandler
	{
	public:
		/**
		 * @brief Constructs a new handler for a connecting client.
		 * @param sock Socket connected to client.
		 * @param storage Implementation of `IServerStorage`.
		 * @param receiver Implementation of `PacketReceiver`.
		 * @param sender Implementation of `PacketSender`.
		 * @note `storage`, `receiver` and `sender` are all assumed to be thread-safe.
		 */
		explicit ConnectingClientHandler(utils::Socket& sock,
										 IServerStorage& storage,
										 PacketReceiver& receiver,
										 PacketSender& sender);

		/**
		 * @brief Connects client.
		 * @return A bool value indicating whether connection (signin) occured,
		 *         and user's username.
		 */
		std::tuple<bool, std::string> connect_client();

	private:
		utils::Socket& _sock;
		IServerStorage& _storage;
		PacketReceiver& _receiver;
		PacketSender& _sender;
	};
}

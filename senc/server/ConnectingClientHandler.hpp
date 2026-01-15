/*********************************************************************
 * \file   ConnectingClientHandler.hpp
 * \brief  Header of ConnectingClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../common/PacketHandler.hpp"
#include "../utils/Socket.hpp"
#include "ServerException.hpp"
#include "IServerStorage.hpp"
#include "ILogger.hpp"
#include <tuple>

namespace senc::server
{
	/**
	 * @class senc::server::ConnectingClientHandler
	 * @brief Handles requests of connecting client (not yet signed in).
	 */
	class ConnectingClientHandler
	{
	public:
		using Self = ConnectingClientHandler;

		/**
		 * @brief Constructs a new handler for a connecting client.
		 * @param logger Implementation of `ILogger` for logging server messages.
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param storage Implementation of `IServerStorage`.
		 * @note `storage` and `packetHandler` are assumed to be thread-safe.
		 */
		explicit ConnectingClientHandler(ILogger& logger,
										 PacketHandler& packetHandler,
										 IServerStorage& storage);

		/**
		 * @brief Connects client.
		 * @return A bool value indicating whether connection (signin) occured,
		 *         and user's username.
		 */
		std::tuple<bool, std::string> connect_client();

	private:
		ILogger& _logger;
		PacketHandler& _packetHandler;
		IServerStorage& _storage;

		enum class Status { Error, Disconnected, Connected };

		/**
		 * @brief Handles signup request.
		 * @param signup Request to handle.
		 * @return Connection handle status and client's user name (if connected).
		 */
		std::tuple<Status, std::string> handle_request(const pkt::SignupRequest signup);

		/**
		 * @brief Handles login request.
		 * @param login Request to handle.
		 * @return Connection handle status and client's user name (if connected).
		 */
		std::tuple<Status, std::string> handle_request(const pkt::LoginRequest login);

		/**
		 * @brief Handles logout request.
		 * @param logout Request to handle.
		 * @return Connection handle status and client's user name (if connected).
		 */
		std::tuple<Status, std::string> handle_request(const pkt::LogoutRequest logout);
	};
}

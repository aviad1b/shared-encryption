/*********************************************************************
 * \file   ConnectingClientHandler.hpp
 * \brief  Header of ConnectingClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../../common/PacketHandler.hpp"
#include "../storage/IServerStorage.hpp"
#include "../loggers/ILogger.hpp"
#include "../ServerException.hpp"
#include <tuple>

namespace senc::server::handlers
{
	/**
	 * @class senc::server::handlers::ConnectingClientHandler
	 * @brief Handles requests of connecting client (not yet signed in).
	 */
	class ConnectingClientHandler
	{
	public:
		using Self = ConnectingClientHandler;

		enum class Status { Error, Disconnected, Connected };

		/**
		 * @brief Constructs a new handler for a connecting client.
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param storage Implementation of `IServerStorage`.
		 * @note `storage` and `packetHandler` are assumed to be thread-safe.
		 */
		explicit ConnectingClientHandler(PacketHandler& packetHandler,
										 storage::IServerStorage& storage);

		/**
		 * @brief Runs a single iteration of client conenction loop.
		 * @return Status, and username (if succeeded).
		 */
		std::tuple<Status, std::string> iteration();

	private:
		PacketHandler& _packetHandler;
		storage::IServerStorage& _storage;

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

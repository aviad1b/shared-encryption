/*********************************************************************
 * \file   ConnectedClientHandler.hpp
 * \brief  Header of ConnectedClientHandler class.
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

namespace senc::server
{
	/**
	 * @brief Handles requests of connected client.
	 */
	class ConnectedClientHandler
	{
	public:
		/**
		 * @brief Constructs a new handler for a connected client.
		 * @param sock Socket connected to client.
		 * @param username Connected client's username.
		 */
		explicit ConnectedClientHandler(utils::Socket& sock, const std::string& username);

		/**
		 * @brief Runs client handlign loop.
		 */
		void loop();

	private:
		utils::Socket& _sock;
		const std::string& _username;
	};
}

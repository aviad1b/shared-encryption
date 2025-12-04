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
#include "DecryptionsManager.hpp"
#include "ServerException.hpp"
#include "IServerStorage.hpp"
#include "UpdateManager.hpp"

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
		 * @param storage Implementation of `IServerStorage`.
		 * @param receiver Implementation of `PacketReceiver`.
		 * @param sender Implementation of `PacketSender`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage`, `receiver` and `sender` are all assumed to be thread-safe.
		 */
		explicit ConnectedClientHandler(utils::Socket& sock,
										const std::string& username,
										IServerStorage& storage,
										PacketReceiver& receiver,
										PacketSender& sender,
										UpdateManager& updateManager,
										DecryptionsManager& decryptionsManager);

		/**
		 * @brief Runs client handlign loop.
		 */
		void loop();

	private:
		utils::Socket& _sock;
		const std::string& _username;
		IServerStorage& _storage;
		PacketReceiver& _receiver;
		PacketSender& _sender;
		UpdateManager& _updateManager;
		DecryptionsManager& _decryptionsManager;

		enum class Status { Connected, Disconnected };

		/**
		 * @brief Runs a single iteration of the client loop.
		 */
		Status iteration();

		Status handle_request(const pkt::LogoutRequest& request);

		Status handle_request(const pkt::MakeUserSetRequest& request);

		Status handle_request(const pkt::GetUserSetsRequest& request);

		Status handle_request(const pkt::GetMembersRequest& request);

		Status handle_request(const pkt::DecryptRequest& request);

		Status handle_request(const pkt::UpdateRequest& request);

		Status handle_request(const pkt::DecryptParticipateRequest& request);

		Status handle_request(const pkt::SendDecryptionPartRequest& request);
	};
}

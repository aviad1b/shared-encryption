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
		 * @param schema Decryptions schema to use for decryptions.
		 * @param storage Implementation of `IServerStorage`.
		 * @param receiver Implementation of `PacketReceiver`.
		 * @param sender Implementation of `PacketSender`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage`, `receiver` and `sender` are all assumed to be thread-safe.
		 */
		explicit ConnectedClientHandler(utils::Socket& sock,
										const std::string& username,
										Schema& schema,
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
		Schema& _schema;
		IServerStorage& _storage;
		PacketReceiver& _receiver;
		PacketSender& _sender;
		UpdateManager& _updateManager;
		DecryptionsManager& _decryptionsManager;

		enum class Status { Connected, Disconnected };

		/**
		 * @brief Creates a new userset.
		 * @param owners Usernames of userset's owners.
		 * @param regMembers Usernames of userset's non-owner members.
		 * @param ownersThreshold Decryption threshold for owners layer.
		 * @param regMembersThreshold Decryption threshold for non-owners layer.
		 * @return ID of userset.
		 * @throw ServerException In case of error.
		 */
		UserSetID make_userset(const std::vector<std::string>& owners,
							   const std::vector<std::string>& regMembers,
							   member_count_t ownersThreshold,
							   member_count_t regMembersThreshold);

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

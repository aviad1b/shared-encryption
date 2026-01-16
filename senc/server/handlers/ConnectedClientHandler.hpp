/*********************************************************************
 * \file   ConnectedClientHandler.hpp
 * \brief  Header of ConnectedClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../../common/PacketHandler.hpp"
#include "../managers/DecryptionsManager.hpp"
#include "../storage/IServerStorage.hpp"
#include "../managers/UpdateManager.hpp"
#include "../loggers/ILogger.hpp"
#include "../ServerException.hpp"
#include "../../utils/Socket.hpp"

namespace senc::server::handlers
{
	/**
	 * @class senc::server::handlers::ConnectedClientHandler
	 * @brief Handles requests of connected client.
	 */
	class ConnectedClientHandler
	{
	public:
		using Self = ConnectedClientHandler;

		/**
		 * @brief Constructs a new handler for a connected client.
		 * @param logger Implementation of `ILogger` for logging server messages.
		 * @param packetHandler Implementation of `PacketHandler`.
		 * @param username Connected client's username.
		 * @param schema Decryptions schema to use for decryptions.
		 * @param storage Implementation of `IServerStorage`.
		 * @param updateManager Instance of `UpdateManager`.
		 * @param decryptionsManager Instance of `DecryptionsManager`.
		 * @note `storage` and `packetHandler` are assumed to be thread-safe.
		 */
		explicit ConnectedClientHandler(loggers::ILogger& logger,
										PacketHandler& packetHandler,
										const std::string& username,
										Schema& schema,
										storage::IServerStorage& storage,
										managers::UpdateManager& updateManager,
										managers::DecryptionsManager& decryptionsManager);

		/**
		 * @brief Runs client handlign loop.
		 */
		void loop();

	private:
		loggers::ILogger& _logger;
		PacketHandler& _packetHandler;
		const std::string& _username;
		Schema& _schema;
		storage::IServerStorage& _storage;
		managers::UpdateManager& _updateManager;
		managers::DecryptionsManager& _decryptionsManager;

		enum class Status { Connected, Disconnected };

		/**
		 * @brief Creates a new userset.
		 * @param creator Creator's username.
		 * @param owners Usernames of userset's owners (besides `creator`).
		 * @param regMembers Usernames of userset's non-owner members.
		 * @param ownersThreshold Decryption threshold for owners layer.
		 * @param regMembersThreshold Decryption threshold for non-owners layer.
		 * @return MakeUsersetResponse containing information about created userset (to return to creator).
		 * @throw ServerException In case of error.
		 */
		pkt::MakeUserSetResponse make_userset(const std::string& creator,
											  const std::vector<std::string>& owners,
											  const std::vector<std::string>& regMembers,
											  member_count_t ownersThreshold,
											  member_count_t regMembersThreshold);

		/**
		 * @brief Initiates a decryption operation.
		 * @param usersetID ID of userset under which decryption should be computed.
		 * @param ciphertext Ciphertext to decrypt.
		 * @return Operation ID of initiated decryption operation.
		 */
		OperationID initiate_decryption(const UserSetID& usersetID,
										Ciphertext&& ciphertext);

		/**
		 * @brief Informs participants that operation went from preperation stage to collection stage.
		 * @param opid Operation ID.
		 * @param opPrepRecord Preperation record of operation from decryptions manager.
		 */
		void continue_operation(const OperationID& opid,
								const managers::DecryptionsManager::PrepareRecord& opPrepRecord);

		/**
		 * @brief Informed initiator that operation is complete.
		 * @param opid Operation ID.
		 * @param opCollRecord Collection record of operation from decryptions manager.
		 */
		void finish_operation(const OperationID& opid,
							  managers::DecryptionsManager::CollectedRecord&& opCollRecord);

		/**
		 * @brief Runs a single iteration of the client loop.
		 */
		Status iteration();

		// NOTE: All handle_request methods accept non-const request for
		//       efficiency (being able to move fields out of the requests)

		Status handle_request(pkt::LogoutRequest& request);

		Status handle_request(pkt::MakeUserSetRequest& request);

		Status handle_request(pkt::GetUserSetsRequest& request);

		Status handle_request(pkt::GetMembersRequest& request);

		Status handle_request(pkt::DecryptRequest& request);

		Status handle_request(pkt::UpdateRequest& request);

		Status handle_request(pkt::DecryptParticipateRequest& request);

		Status handle_request(pkt::SendDecryptionPartRequest& request);
	};
}

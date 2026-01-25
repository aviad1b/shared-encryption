/*********************************************************************
 * \file   UpdateManager.hpp
 * \brief  Header of update manager class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../../common/packets.hpp"
#include "../../common/aliases.hpp"
#include "../../utils/hash.hpp"
#include <mutex>

namespace senc::server::managers
{
	/**
	 * @class senc::server::managers::UpdateManager
	 * @brief Managers registry of user updates (before sent).
	 */
	class UpdateManager
	{
	public:
		using Self = UpdateManager;

		UpdateManager() = default;

		/**
		 * @brief Retrieves updates for a specific user.
		 * @param username Username of user to retrieve updates for.s
		 * @return Retireved updates.
		 */
		pkt::UpdateResponse retrieve_updates(const std::string& username);

		/**
		 * @brief Registers that a user was added to a userset as non-owner.
		 * @param username Username of user that was added to userset.
		 * @param usersetID ID of userset to which user was added.
		 * @param regLayerPubKey Non-owner layer public key used in this user userset for encryption.
		 * @param ownerLayerPubKey Owner layer public key used in this user userset for encryption.
		 * @param privKeyShard User's private key shard in userset.
		 */
		void register_reg_member(const std::string& username,
								 const UserSetID& usersetID,
								 const PubKey& regLayerPubKey,
								 const PubKey& ownerLayerPubKey,
								 PrivKeyShard&& privKeyShard);

		/**
		 * @brief Registers that a user was added to a userset as owner.
		 * @param username Username of user that was added to userset.
		 * @param usersetID ID of userset to which user was added.
		 * @param regLayerPubKey Non-owner layer public key used in this user userset for encryption.
		 * @param ownerLayerPubKey Owner layer public key used in this user userset for encryption.
		 * @param regLayerPrivKeyShard User's private key shard in userset for non-owner layer (moved).
		 * @param ownerLayerPrivKeyShard User's private key shard in userset for owner layer (moved).
		 */
		void register_owner(const std::string& username,
							const UserSetID& usersetID,
							const PubKey& regLayerPubKey,
							const PubKey& ownerLayerPubKey,
							PrivKeyShard&& regLayerPrivKeyShard,
							PrivKeyShard&& ownerLayerPrivKeyShard);

		/**
		 * @brief Registers user to look for in order to perform a decryption operation.
		 * @param username Username of user to include in lookup for operation.
		 * @param opid Operation ID.
		 */
		void register_lookup(const std::string& username, const OperationID& opid);

		/**
		 * @brief Registers a user's participance in a decryption operation.
		 * @param username Username of user participating in decryption.
		 * @param opid Operation ID.
		 * @param ciphertext Ciphertext being decrypted.
		 * @param shardsIDs IDs of key shards used in decryption.
		 */
		void register_decryption_participating(const std::string& username,
											   const OperationID& opid,
											   const Ciphertext& ciphertext,
											   const std::vector<PrivKeyShardID>& shardsIDs);

		/**
		 * @brief Registers a finished decryption operation.
		 * @param username Username of user who initiated the operation.
		 * @param opid Operation ID.
		 * @param regLayerParts Decryption parts for non-owner layer (moved).
		 * @param ownerLayerParts Decryption parts for owner layer (moved).
		 * @param regLayerShardsIDs Shards IDs used in non-owner layer (moved).
		 * @param ownerLayerShardsIDs Shards IDs used in owner layer (moved).
		 */
		void register_finished_decrpytion(const std::string& username,
										  const OperationID& opid,
										  std::vector<DecryptionPart>&& regLayerParts,
										  std::vector<DecryptionPart>&& ownerLayerParts,
										  std::vector<PrivKeyShardID>&& regLayerShardsIDs,
										  std::vector<PrivKeyShardID>&& ownerLayerShardsIDs);

	private:
		// maps username to updates prepared so far
		utils::HashMap<std::string, pkt::UpdateResponse> _updates;
		std::mutex _mtxUpdates;
	};
}

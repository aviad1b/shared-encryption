/*********************************************************************
 * \file   UpdateManager.hpp
 * \brief  Header of update manager class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../common/packets.hpp"
#include "../common/aliases.hpp"
#include "../utils/hash.hpp"
#include <mutex>

namespace senc::server
{
	/**
	 * @class senc::server::UpdateManager
	 * @brief Managers registry of user updates (before sent).
	 */
	class UpdateManager
	{
	public:
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
		 * @param pubKey1 First public key used in this user userset for encryption (moved).
		 * @param pubKey2 Second public key used in this user userset for encryption (moved).
		 * @param privKeyShard User's private key shard in userset (moved).
		 */
		void register_reg_member(const std::string& username,
								 const UserSetID& usersetID,
								 PubKey&& pubKey1, PubKey&& pubKey2,
								 PrivKeyShard&& privKeyShard);

		/**
		 * @brief Registers that a user was added to a userset as owner.
		 * @param username Username of user that was added to userset.
		 * @param usersetID ID of userset to which user was added.
		 * @param pubKey1 First public key used in this user userset for encryption (moved).
		 * @param pubKey2 Second public key used in this user userset for encryption (moved).
		 * @param privKeyShard1 User's private key shard in userset for layer1 (moved).
		 * @param privKeyShard2 User's private key shard in userset for layer2 (moved).
		 */
		void register_owner(const std::string& username,
							const UserSetID& usersetID,
							PubKey&& pubKey1, PubKey&& pubKey2,
							PrivKeyShard&& privKeyShard1, PrivKeyShard&& privKeyShard2);

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
		void register_decryption_participated(const std::string& username,
											  const OperationID& opid,
											  const Ciphertext& ciphertext,
											  const std::vector<PrivKeyShardID>& shardsIDs);

		/**
		 * @brief Registers a finished decryption operation.
		 * @param username Username of user who initiated the operation.
		 * @param opid Operation ID.
		 * @param usersetID ID of userset under which decrption occured.
		 * @param parts1 Decryption parts for first layer (moved).
		 * @param parts2 Decryption parts for second layer (moved).
		 */
		void register_finished_decrpytion(const std::string& username,
										  const OperationID& opid,
										  const UserSetID& usersetID,
										  std::vector<DecryptionPart>&& parts1,
										  std::vector<DecryptionPart>&& parts2);

	private:
		// maps username to updates prepared so far
		utils::HashMap<std::string, pkt::UpdateResponse> _updates;
		std::mutex _mtxUpdates;
	};
}

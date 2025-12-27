/*********************************************************************
 * \file   DecryptionsManager.hpp
 * \brief  Header of DecryptionsManager class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../common/aliases.hpp"
#include "../common/sizes.hpp"
#include "../utils/hash.hpp"
#include <optional>
#include <mutex>

namespace senc::server
{
	/**
	 * @class senc::server::DecryptionsManager
	 * @brief Manages synchronized decryption operations.
	 */
	class DecryptionsManager
	{
	public:
		using Self = DecryptionsManager;

		/**
		 * @brief Record for collected parts of an operation.
		 */
		struct CollectedRecord
		{
			std::string requester;
			UserSetID userset_id;
			member_count_t required_owners;
			member_count_t required_reg_members;
			std::vector<DecryptionPart> parts1;
			std::vector<PrivKeyShardID> shardsIDs1;
			std::vector<DecryptionPart> parts2;
			std::vector<PrivKeyShardID> shardsIDs2;

			CollectedRecord(const std::string& requester,
							const UserSetID& usersetID,
							member_count_t requiredOwners,
							member_count_t requiredRegMembers)
				: requester(requester),
				  userset_id(usersetID),
				  required_owners(requiredOwners),
				  required_reg_members(requiredRegMembers) { }

			bool has_enough_parts() const;
		};

		/**
		 * @brief Record for a preperation of an operation (lookup).
		 */
		struct PrepareRecord
		{
			std::string requester;
			UserSetID userset_id;
			Ciphertext ciphertext;
			member_count_t required_owners;
			member_count_t required_reg_members;
			utils::HashSet<std::string> owners_found;
			utils::HashSet<std::string> reg_members_found;

			PrepareRecord(const std::string& requester,
						  const UserSetID& usersetID,
						  Ciphertext&& ciphertext,
						  member_count_t requiredOwners,
						  member_count_t requiredRegMembers)
				: requester(requester),
				  userset_id(usersetID),
				  ciphertext(std::move(ciphertext)),
				  required_owners(requiredOwners),
				  required_reg_members(requiredRegMembers) { }

			bool has_enough_participants() const;
		};

		/**
		 * @brief Generates an operation ID for a new operation.
		 * @return Unique operation ID.
		 */
		OperationID new_operation();

		/**
		 * @brief Prepares a decryption operation.
		 * @param opid Operation ID.
		 * @param requester Username of requesting user.
		 * @param usersetID ID of userset under which decryption is performed.
		 * @param ciphertext Ciphertext being decrypted (moved).
		 * @param requiredOwners Amount of owners required for performing the decryption.
		 * @param requiredRegMembers Amount of non-owner members required for performing the decryption.
		 */
		void prepare_operation(const OperationID& opid,
							   const std::string& requester,
							   const UserSetID& usersetID,
							   Ciphertext&& ciphertext,
							   member_count_t requiredOwners,
							   member_count_t requiredRegMembers);

		/**
		 * @brief Registers a client that is willing to aprticipate in an operation.
		 * @param opid Operation ID.
		 * @param username Participant's username.
		 * @param isOwner Whether or not client is of an owner.
		 * @return 1. Record of prepared operation if has enough members, `std::nullopt` otherwise.
		 *		   2. `true` if user was required for decryption, otherwise `false`.
		 */
		std::pair<std::optional<PrepareRecord>, bool> register_participant(const OperationID& opid,
																		   const std::string& username,
																		   bool isOwner);

		/**
		 * @brief Registers a decryption part provided by a member.
		 * @param opid Decryption operation ID.
		 * @param part Part provided by member.
		 * @param shardID ID of shard used for providing this part.
		 * @param isOwner Whether or not this is an owner's part.
		 * @return Record of collected parts if collecting completed, `std::nullopt` otherwise.
		 */
		std::optional<CollectedRecord> register_part(const OperationID& opid,
													 DecryptionPart&& part,
													 PrivKeyShardID&& shardID,
													 bool isOwner);

		/**
		 * @brief Gets userset of operation.
		 * @param opid Operation ID.
		 * @return ID of userset under which operation is performed.
		 */
		const UserSetID get_operation_userset(const OperationID& opid);

	private:
		utils::HashMap<OperationID, PrepareRecord> _prep;
		std::mutex _mtxPrep;

		utils::HashMap<OperationID, CollectedRecord> _collected;
		std::mutex _mtxCollected;

		utils::HashSet<OperationID> _allOpIDs;
		std::mutex _mtxAllOpIDs;
	};
}

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
			std::vector<DecryptionPart> parts1;
			std::vector<PrivKeyShardID> shardsIDs1;
			std::vector<DecryptionPart> parts2;
			std::vector<PrivKeyShardID> shardsIDs2;
		};

		/**
		 * @brief Record for a preperation of an operation (lookup).
		 */
		struct PrepareRecord
		{
			Ciphertext ciphertext;
			member_count_t required_owners;
			member_count_t required_reg_members;
			utils::HashSet<std::string> owners_found;
			utils::HashSet<std::string> reg_members_found;

			PrepareRecord(Ciphertext&& ciphertext,
						  member_count_t requiredOwners,
						  member_count_t requiredRegMembers)
				: ciphertext(std::move(ciphertext)),
				  required_owners(requiredOwners),
				  required_reg_members(requiredRegMembers) { }
		};

		/**
		 * @brief Registers a new decryption operation.
		 * @param opid Operation ID.
		 * @param requester Username of requesting user.
		 * @param ciphertext Ciphertext being decrypted (moved).
		 * @param requiredOwners Amount of owners required for performing the decryption.
		 * @param requiredRegMembers Amount of non-owner members required for performing the decryption.
		 */
		void register_operation(const OperationID& opid,
								const std::string& requester,
								Ciphertext&& ciphertext,
								member_count_t requiredOwners,
								member_count_t requiredRegMembers);

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

	private:
		utils::HashMap<OperationID, PrepareRecord> _prep;
		std::mutex _mtxPrep;

		utils::HashMap<OperationID, CollectedRecord> _collected;
		std::mutex _mtxCollected;
	};
}

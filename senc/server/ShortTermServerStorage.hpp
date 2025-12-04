/*********************************************************************
 * \file   ShortTermServerStorage.hpp
 * \brief  Header of ShortTermServerStorage class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "IServerStorage.hpp"
#include <mutex>

namespace senc::server
{
	/**
	 * @class senc::server::ShortTermServerStorage
	 * @brief Implementation of `IServerStorage` which uses runtime memory only.
	 */
	class ShortTermServerStorage : public IServerStorage
	{
	public:
		using Self = ShortTermServerStorage;

		ShortTermServerStorage();

		void new_user(const std::string& username) override;

		bool user_exists(const std::string& username) override;

		UserSetID new_userset(const utils::HashSet<std::string>& owners,
							  const utils::HashSet<std::string>& regMembers,
							  member_count_t ownersThreshold,
							  member_count_t regMembersThreshold) override;

		utils::HashSet<UserSetID> get_usersets(const std::string& owner) override;

		bool user_owns_userset(const std::string& user, const UserSetID& userset) override;

		UserSetInfo get_userset_info(const UserSetID& userset) override;

		PrivKeyShardID get_shard_id(const std::string& user, const UserSetID& userset) override;

	private:
		utils::Distribution<PrivKeyShardID> _shardsDist;

		// map user to owned sets
		std::mutex _mtxUsers;
		utils::HashMap<std::string, utils::HashSet<UserSetID>> _users;

		// map userset to info
		std::mutex _mtxUsersets;
		utils::HashMap<UserSetID, UserSetInfo> _usersets;

		// map opid to operation info
		std::mutex _mtxOperations;
		utils::HashMap<OperationID, OperationInfo> _operations;

		// map user and userset to shard ID
		std::mutex _mtxShardIDs;
		utils::HashMap<UserSetID, utils::HashSet<PrivKeyShardID>> _usersetShardIDs;
		utils::HashMap<std::tuple<std::string, UserSetID>, PrivKeyShardID> _shardIDs;
	};
}

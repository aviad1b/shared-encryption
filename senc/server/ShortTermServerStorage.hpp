/*********************************************************************
 * \file   ShortTermServerStorage.hpp
 * \brief  Header of ShortTermServerStorage class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "IServerStorage.hpp"
#include "aliases.hpp"
#include <mutex>
#include <set>

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

		void new_user(const std::string& username, const std::string& password) override;

		bool user_exists(const std::string& username) override;

		bool user_has_password(const std::string& username, const std::string& password) override;

		UserSetID new_userset(utils::ranges::StringViewRange&& owners,
							  utils::ranges::StringViewRange&& regMembers,
							  member_count_t ownersThreshold,
							  member_count_t regMembersThreshold) override;

		std::vector<UserSetID> get_usersets(const std::string& owner) override;

		bool user_owns_userset(const std::string& user, const UserSetID& userset) override;

		UserSetInfo get_userset_info(const UserSetID& userset) override;

		PrivKeyShardID get_shard_id(const std::string& user, const UserSetID& userset) override;

	private:
		struct StoredUserSetInfo
		{
			std::set<std::string> owners;
			std::set<std::string> reg_members;
			member_count_t owners_threshold;
			member_count_t reg_members_threshold;
		};

		utils::Distribution<PrivKeyShardID> _shardsDist;

		PwdHasher _pwdHasher;

		PrivKeyShardID sample_shard_id(const utils::HasContainsMethod<PrivKeyShardID> auto& container)
		{
			return _shardsDist(container);
			// no need to check for non-zero, since distribution is now confined above 0.
			// if the confining range ever changes to include zero, a check against a 
			// zero-value shard ID should be done here.
		}

		// map user to password salt, password hash and owned sets
		struct UserRecord
		{
			PwdSalt pwd_salt;
			PwdHash pwd_hash;
			std::set<UserSetID> usersets;

			// constructs user record from password (username is map key)
			UserRecord(PwdHasher& pwdHasher, const std::string& password)
				: pwd_salt(pwdHasher.generate_salt()),
				  pwd_hash(pwdHasher.hash(password, pwd_salt)) { }
		};
		std::mutex _mtxUsers;
		utils::HashMap<std::string, UserRecord> _users;

		// map userset to info
		std::mutex _mtxUsersets;
		utils::HashMap<UserSetID, StoredUserSetInfo> _usersets;

		// map user and userset to shard ID
		std::mutex _mtxShardIDs;
		utils::HashMap<UserSetID, utils::HashSet<PrivKeyShardID>> _usersetShardIDs;
		utils::HashMap<std::tuple<std::string, UserSetID>, PrivKeyShardID> _shardIDs;
	};
}

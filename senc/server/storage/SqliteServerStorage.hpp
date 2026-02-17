/*********************************************************************
 * \file   SqliteServerStorage.hpp
 * \brief  Header of SqliteServerStorage class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "../../utils/sqlite/Database.hpp"
#include "IServerStorage.hpp"
#include <mutex>

namespace senc::server::storage
{
	/**
	 * @class senc::server::storage::SqliteServerStorage
	 * @brief Implementation of `IServerStorage` which uses an SQLite database.
	 */
	class SqliteServerStorage : public IServerStorage
	{
	public:
		using Self = SqliteServerStorage;

		SqliteServerStorage(const std::string& path);

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
		
	};
}

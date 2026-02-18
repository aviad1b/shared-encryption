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
#include "../aliases.hpp"
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

		/**
		 * @brief Constructs a new SQLite server storage instance.
		 * @param path Path to SQLite database file.
		 */
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
		// Schema:
		// Users(username TEXT PK, pwd_salt BLOB, pwd_hash BLOB)
		// UserSets(id PK BLOB, owners_threshold INT, reg_members_threshold INT)
		// Members(username TEXT FK[Users.username], userset_id BLOB FK[UserSets.id], shard_id BLOB, is_owner INT)
		utils::sqlite::Database<utils::sqlite::schemas::DB<
			utils::sqlite::schemas::Table<"Users",
				utils::sqlite::schemas::PrimaryKey<"username", utils::sqlite::Text>,
				utils::sqlite::schemas::Col       <"pwd_salt", utils::sqlite::Blob>,
				utils::sqlite::schemas::Col       <"pwd_hash", utils::sqlite::Blob>
			>,
			utils::sqlite::schemas::Table<"UserSets",
				utils::sqlite::schemas::PrimaryKey<"id"                   , utils::sqlite::Blob>,
				utils::sqlite::schemas::Col       <"owners_threshold"     , utils::sqlite::Int >,
				utils::sqlite::schemas::Col       <"reg_members_threshold", utils::sqlite::Int >
			>,
			utils::sqlite::schemas::Table<"Members",
				utils::sqlite::schemas::ForeignKey<"username"  , utils::sqlite::Text, "Users"         >,
				utils::sqlite::schemas::ForeignKey<"userset_id", utils::sqlite::Blob, "UserSets", "id">,
				utils::sqlite::schemas::Col       <"shard_id"  , utils::sqlite::Blob                  >,
				utils::sqlite::schemas::Col       <"is_owner"  , utils::sqlite::Int                   >
			>
		>> _db;
		std::mutex _mtxDB;

		utils::Distribution<PrivKeyShardID> _shardsDist;
		PwdHasher _pwdHasher;

		/**
		 * @brief Checks if a userset ID is of an existing userset.
		 * @param usersetID Userset ID to check if is of an existing userset.
		 * @return `true` if `usersetID` is an ID of an existing userset, otherwise `false`.
		 */
		bool userset_exists(const UserSetID& usersetID);

		/**
		 * @brief Generates a unique userset ID.
		 * @return Generated userset ID.
		 */
		UserSetID generate_unique_userset_id();

		/**
		 * @brief Checks if a shard ID exists within a userset.
		 * @param shardID Shard ID to check if exists, in the form of a blob view.
		 * @param usersetID Userset ID.
		 * @return `true` if `shardID` exists in userset with ID `usersetID`, otherwise `false`.
		 */
		bool shard_id_exists(const PrivKeyShardID& shardID, const UserSetID& usersetID);

		/**
		 * @brief Generates a unique shard ID for a userset member.
		 * @param usersetID ID of userset.
		 * @return Generated shard ID.
		 */
		PrivKeyShardID generate_unique_shard_id(const UserSetID& usersetID);
	};
}

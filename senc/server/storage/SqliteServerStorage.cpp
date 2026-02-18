/*********************************************************************
 * \file   SqliteServerStorage.cpp
 * \brief  Implementation of SqliteServerStorage class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "SqliteServerStorage.hpp"

namespace sql = senc::utils::sqlite;

namespace senc::server::storage
{
	SqliteServerStorage::SqliteServerStorage(const std::string& path)
		: _db(path),
		  _shardsDist(utils::Random<PrivKeyShardID>::get_range_dist(1, MAX_MEMBERS)) { }

	void SqliteServerStorage::new_user(const std::string& username, const std::string& password)
	{
		if (user_exists(username))
			throw UserExistsException(username);

		auto pwdSalt = _pwdHasher.generate_salt();
		auto pwdHash = _pwdHasher.hash(password, pwdSalt);

		const std::lock_guard<std::mutex> lock(_mtxDB);
		this->_db.insert<"Users">(sql::TextView(username), sql::BlobView(pwdSalt), sql::BlobView(pwdHash));
	}

	bool SqliteServerStorage::user_exists(const std::string& username)
	{
		bool found = false;
		const std::lock_guard<std::mutex> lock(_mtxDB);
		this->_db.select<"Users", sql::SelectArg<"username">>()
			.where("username = " + sql::Text(username).as_sqlite())
			>> [&found](const sql::TextView&) { found = true; };
		return found;
	}

	bool SqliteServerStorage::user_has_password(const std::string& username, const std::string& password)
	{
		bool found = false;
		PwdSalt pwdSalt{};
		PwdHash pwdHash{};

		{
			const std::lock_guard<std::mutex> lock(_mtxDB);
			this->_db.select<"Users", sql::SelectArg<"pwd_salt">, sql::SelectArg<"pwd_hash">>()
				.where("username = " + sql::Text(username).as_sqlite())
				>> [&found, &pwdSalt, &pwdHash](sql::BlobView salt, sql::BlobView hash)
				{
					// TODO: Replace memcpy calls with direct call once pwdhash supports views
					std::memcpy(pwdSalt.data(), salt.get().data(), std::min(salt.get().size(), pwdSalt.size()));
					std::memcpy(pwdHash.data(), hash.get().data(), std::min(hash.get().size(), pwdHash.size()));
					found = true;
				};
		}

		// return true iff hash on input equals to stored hash
		const auto inputPwdHash = _pwdHasher.hash(password, pwdSalt);
		return inputPwdHash == pwdHash;
	}

	UserSetID SqliteServerStorage::new_userset(utils::ranges::StringViewRange&& owners,
											   utils::ranges::StringViewRange&& regMembers,
											   member_count_t ownersThreshold,
											   member_count_t regMembersThreshold)
	{
		// check if all members exist
		for (const auto& member : utils::views::join(owners, regMembers))
			if (!user_exists(member))
				throw UserNotFoundException(member);

		// generate set ID and insert new userset
		const auto setID = generate_unique_userset_id();
		const sql::BlobView setIDBlobView(setID.data(), setID.size());
		{
			const std::lock_guard<std::mutex> lock(_mtxDB);
			this->_db.insert<"UserSets">(
				setIDBlobView,
				sql::Int(ownersThreshold),
				sql::Int(regMembersThreshold)
			);
		}

		// register shard IDs for all members
		auto markedMembers = utils::views::join(
			owners | std::views::transform([](auto&& x) { return std::make_pair(x, true); }),
			regMembers | std::views::transform([](auto&& x) { return std::make_pair(x, false); })
		);
		for (const auto& [member, isOwner] : markedMembers)
		{
			auto shardID = generate_unique_shard_id(setID);

			utils::Buffer shardIDBytes(shardID.MinEncodedSize());
			shardID.Encode(shardIDBytes.data(), shardIDBytes.size());

			const std::unique_lock<std::mutex> lock(_mtxDB);
			this->_db.insert<"Members">(
				sql::TextView(member),
				setIDBlobView,
				sql::BlobView(shardIDBytes),
				sql::Int(isOwner)
			);
		}

		return setID;
	}
}

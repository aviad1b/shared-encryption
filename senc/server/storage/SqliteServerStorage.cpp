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
		this->_db.insert<"Users">(sql::Text(username), sql::BlobView(pwdSalt), sql::BlobView(pwdHash));
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
}

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

		this->_db.insert<"Users">(sql::Text(username), sql::BlobView(pwdSalt), sql::BlobView(pwdHash));
	}

	bool SqliteServerStorage::user_exists(const std::string& username)
	{
		bool found = false;
		this->_db.select<"Users", sql::SelectArg<"username">>()
			.where("username = " + sql::Text(username).as_sqlite())
			>> [&found](const sql::TextView&) { found = true; };
		return found;
	}
}

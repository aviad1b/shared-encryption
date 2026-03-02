/*********************************************************************
 * \file   aliases.hpp
 * \brief  Contains type aliases for server.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/pwd/PBKDF2.hpp"

namespace senc::server
{
	/**
	 * @typedef senc::server::PwdHasher
	 * @brief Hasher used for hashing passwords.
	 */
	using PwdHasher = utils::pwd::PBKDF2<>;

	/**
	 * @typedef senc::server::PwdSalt
	 * @brief Type of password hasher's salt values.
	 */
	using PwdSalt = utils::pwd::Salt<PwdHasher>;

	/**
	 * @typedef senc::server::PwdHash
	 * @brief Type of password hasher's output.
	 */
	using PwdHash = utils::pwd::PwdHash<PwdHasher>;
}

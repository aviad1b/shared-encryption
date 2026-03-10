/*********************************************************************
 * \file   ClientUtils.hpp
 * \brief  Header of utilities for client.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include <string>

namespace senc::clientapi
{
	/**
	 * @class senc::clientapi::ClientUtils
	 * @brief Contains utilities for client.
	 */
	class ClientUtils
	{
	public:
		/**
		 * @brief Locates user's profile file path based on username.
		 * @param username Username.
		 * @param profileBaseDir Base directory where local profile data should be stored.
		 * @return User profile file path.
		 */
		static std::string locate_user_profile_file(const std::string& username,
													const std::string& profileBaseDir);
	};
}

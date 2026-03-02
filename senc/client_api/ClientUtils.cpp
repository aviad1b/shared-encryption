/*********************************************************************
 * \file   ClientUtils.cpp
 * \brief  Implementation of utilities for client.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "ClientUtils.hpp"

namespace senc::clientapi
{
	std::string ClientUtils::locate_user_profile_file(const std::string& username)
	{
		return username + ".sencp";
	}
}

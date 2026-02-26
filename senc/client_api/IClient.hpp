/*********************************************************************
 * \file   IClient.hpp
 * \brief  Contains `IClient` interface.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "../common/aliases.hpp"
#include "../utils/ranges.hpp"
#include <functional>

namespace senc::clientapi
{
	/**
	 * @interface senc::clientapi::IClient
	 * @brief Interface of client functionality.
	 */
	class IClient
	{
	public:
		virtual ~IClient() { }

		virtual void signup(const std::string& username, const std::string& password) = 0;

		virtual void login(const std::string& username, const std::string& password) = 0;

		virtual void logout() = 0;

		virtual void make_userset(utils::ranges::StringViewRange&& owners,
								  utils::ranges::StringViewRange&& regMembers,
								  member_count_t ownersThreshold,
								  member_count_t regMembersThreshold) = 0;

		virtual void get_usersets(std::function<void(const UserSetID&)> callback) = 0;

		virtual void get_userset_members(const UserSetID& usersetID,
										 std::function<void(const std::string&)> callback) = 0;

		virtual utils::Buffer encrypt(const utils::Buffer& msg) = 0;

		virtual OperationID decrypt(const UserSetID& usersetID, const utils::Buffer& ciphertext) = 0;
	};
}

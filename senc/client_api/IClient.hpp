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

		/**
		 * @brief Signs up as a new username (and stays logged in).
		 * @param username Username to signup with.
		 * @param password Password to signup with.
		 */
		virtual void signup(const std::string& username, const std::string& password) = 0;

		/**
		 * @brief Logs in to server (and stays logged in).
		 * @param username Username to login with.
		 * @param password Password to login with.
		 */
		virtual void login(const std::string& username, const std::string& password) = 0;

		/**
		 * @brief Logs out of server.
		 */
		virtual void logout() = 0;

		/**
		 * @brief Iterates over user's profile records.
		 * @note Requires user to be logged in.
		 * @param callback Callback function accepting current profile data record, returning `false` to stop.
		 */
		virtual void iter_profile(std::function<bool(const storage::ProfileRecord&)> callback) = 0;

		/**
		 * @brief Creates a new userset.
		 * @note Requires user to be logged in.
		 * @param owners Usernames of userset's owner members.
		 * @param regMembers Usernames of userset's non-owner members.
		 * @param ownersThreshold Minimum amount of owners required for decryption.
		 * @param regMembersThreshold Minimum amount of non-owners required for decryption.
		 * @param name Userset name (moved).
		 * @return Userset's ID.
		 */
		virtual UserSetID make_userset(utils::ranges::StringViewRange&& owners,
									   utils::ranges::StringViewRange&& regMembers,
									   member_count_t ownersThreshold,
									   member_count_t regMembersThreshold,
									   std::string&& name) = 0;

		/**
		 * @brief Gets all usersets owned by user.
		 * @note Requires user to be logged in.
		 * @param callback Callback function to call on ID and name of each userset owned by user.
		 */
		virtual void get_usersets(std::function<void(const UserSetID&, const std::string&)> callback) = 0;

		/**
		 * @brief Gets all members of a specific userset.
		 * @note Requires user to be logged in.
		 * @param usersetID ID of userset to get members of.
		 * @param ownersCallback Callback function to call on username of each owner member.
		 * @param regsCallback Callback function to call on username of each non-owner member.
		 */
		virtual void get_userset_members(const UserSetID& usersetID,
										 std::function<void(const std::string&)> ownersCallback,
										 std::function<void(const std::string&)> regsCallback) = 0;

		/**
		 * @brief Encrypts a message under a userset.
		 * @note Requires user to be logged in.
		 * @param usersetID ID of userset to encrypt under.
		 * @param msg Message to encrypt.
		 * @return Encrypted message.
		 */
		virtual Ciphertext encrypt(const UserSetID& usersetID, const utils::Buffer& msg) = 0;

		/**
		 * @brief Queues a message decryption under a userset.
		 * @note Requires user to be logged in.
		 * @param usersetID ID of userset to decrypt under.
		 * @param ciphertext Encrypted message to decrypt.
		 * @return Decryption operation ID.
		 */
		virtual OperationID decrypt(const UserSetID& usersetID, const Ciphertext& ciphertext) = 0;

		/**
		 * @brief Forces client update.
		 */
		virtual void force_update() = 0;
	};
}

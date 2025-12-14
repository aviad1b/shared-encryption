/*********************************************************************
 * \file   IServerStorage.hpp
 * \brief  Contains definition of IServerStorage interface.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../common/aliases.hpp"
#include "../common/sizes.hpp"
#include "../utils/hash.hpp"
#include "ServerException.hpp"
#include <string>
#include <vector>

namespace senc::server
{
	class ServerStorageException : public ServerException
	{
	public:
		using Self = ServerStorageException;
		using Base = ServerException;

		ServerStorageException(const std::string& msg) : Base(msg) {}

		ServerStorageException(std::string&& msg) : Base(std::move(msg)) {}

		ServerStorageException(const std::string& msg, const std::string& info) : Base(msg, info) {}

		ServerStorageException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) {}

		ServerStorageException(const Self&) = default;

		Self& operator=(const Self&) = default;

		ServerStorageException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};

	/**
	 * @class senc::server::UserExistsException
	 * @brief Type of exceptions thrown by server storage when
	 *        trying to make user that already exists.
	 */
	class UserExistsException : public ServerStorageException
	{
	public:
		using Self = UserExistsException;
		using Base = ServerStorageException;

		UserExistsException(const std::string& username)
			: Base("User \"" + username + "\" already exists"), _username(username) { }

		UserExistsException(const Self&) = default;

		Self& operator=(const Self&) = default;

		UserExistsException(Self&&) = default;

		Self& operator=(Self&&) = default;

		const std::string& username() const { return _username; }

	private:
		std::string _username;
	};

	/**
	 * @struct senc::server::UserSetInfo
	 * @brief Used for holding/returning information about a userset.
	 */
	struct UserSetInfo
	{
		utils::HashSet<std::string> owners;
		utils::HashSet<std::string> reg_members;
		member_count_t owners_threshold;
		member_count_t reg_members_threshold;
	};

	/**
	 * @struct senc::server::OperationInfo
	 * @brief Used for holding/returning information about a requested operation.
	 */
	struct OperationInfo
	{
		std::string requester;
		bool is_completed = false;
	};

	/**
	 * @interface senc::server::IServerStorage
	 * @brief Interface of server storage.
	 */
	class IServerStorage
	{
	public:
		virtual ~IServerStorage() { }

		/**
		 * @brief Registers new user.
		 * @param username Username for user.
		 * @throw UserExistsException If user already exists.
		 * @throw ServerStorageException In case of other errors.
		 */
		virtual void new_user(const std::string& username) = 0;

		/**
		 * @brief Checks if a user with a given username exists.
		 * @param username Username to check if exists.
		 * @return `true` if user with username `username` exists, otherwise `false`.
		 * @throw ServerStorageException In case of error.
		 */
		virtual bool user_exists(const std::string& username) = 0;

		/**
		 * @brief Registers a new userset.
		 * @param owners Usernames of userset's owners.
		 * @param regMembers Usernames of userset's non-owner members.
		 * @param ownersThreshold Decryption threshold for owners layer.
		 * @param regMembersThreshold Decryption threshold for non-owners layer.
		 * @return ID of userset.
		 * @throw ServerStorageException In case of error.
		 */
		virtual UserSetID new_userset(const utils::HashSet<std::string>& owners,
									  const utils::HashSet<std::string>& regMembers,
									  member_count_t ownersThreshold,
									  member_count_t regMembersThreshold) = 0;

		/**
		 * @brief Gets all usersets owned by a specific user.
		 * @param owner Username of user to get usersets owned by it.
		 * @return Set of IDs of usersets where `owner` is an owner.
		 * @throw ServerStorageException In case of error.
		 */
		virtual utils::HashSet<UserSetID> get_usersets(const std::string& owner) = 0;

		/**
		 * @brief Checks if a given user owns a given userset.
		 * @param user Username of user to check if owns a specific userset.
		 * @param userset ID of userset to check if `user` owns.
		 * @return `true` if `user` owns `userset`, otherwise `false`.
		 * @throw ServerStorageException In case of error.
		 */
		virtual bool user_owns_userset(const std::string& user, const UserSetID& userset) = 0;

		/**
		 * @brief Gets information about userset (userset configuration).
		 * @param userset ID of userset.
		 * @return Information about userset.
		 * @throw ServerException In case of error.
		 */
		virtual UserSetInfo get_userset_info(const UserSetID& userset) = 0;

		/**
		 * @brief Gets a given user's shard ID under a given userset.
		 * @param user Username of user to get its shard ID.
		 * @param userset ID of userset.
		 * @return Shard ID of `user` under `userset`.
		 * @throw ServerStorageException In case of error.
		 */
		virtual PrivKeyShardID get_shard_id(const std::string& user, const UserSetID& userset) = 0;
	};
}

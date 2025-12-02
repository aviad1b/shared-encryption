/*********************************************************************
 * \file   ShortTermServerStorage.cpp
 * \brief  Implementation of ShortTermServerStorage class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ShortTermServerStorage.hpp"

#include "../utils/ranges.hpp"

namespace senc::server
{
	ShortTermServerStorage::ShortTermServerStorage()
		: _shardsDist(utils::Random<PrivKeyShardID>::get_dist()) { }

	OperationID ShortTermServerStorage::new_operation(const std::string& requester)
	{
		const std::lock_guard<std::mutex> lock(_mtxOperations);
		auto opid = OperationID::generate(_operations);
		_operations.insert(std::make_pair(opid, requester));
		return opid;
	}

	OperationInfo ShortTermServerStorage::get_operation_info(const OperationID& opid)
	{
		const std::lock_guard<std::mutex> lock(_mtxOperations);
		const auto it = _operations.find(opid);
		if (it == _operations.end())
			throw ServerException("No operation with ID " + opid.to_string());
		return it->second;
	}

	void ShortTermServerStorage::new_user(const std::string& username)
	{
		bool inserted = false;
		{
			const std::lock_guard<std::mutex> lock(_mtxUsers);
			inserted = _users.insert(std::make_pair(
				username, utils::HashSet<UserSetID>{}
			)).second;
		}
		if (!inserted)
			throw ServerException("Username " + username + " already exists");
	}

	bool ShortTermServerStorage::user_exists(const std::string& username)
	{
		const std::lock_guard<std::mutex> lock(_mtxUsers);
		return _users.contains(username);
	}

	UserSetID ShortTermServerStorage::new_userset(const utils::HashSet<std::string>& owners,
												  const utils::HashSet<std::string>& regMembers,
												  member_count_t ownersThreshold,
												  member_count_t regMembersThreshold)
	{
		UserSetID setID;

		// insert new userset to map
		{
			const std::lock_guard<std::mutex> lock(_mtxUsersets);
			setID = UserSetID::generate(_usersets);
			_usersets.insert(std::make_pair(
				setID,
				UserSetInfo{
					owners,
					regMembers,
					ownersThreshold,
					regMembersThreshold
				}
			));
		}

		// check if all members exist
		for (const auto& member : utils::views::join(owners, regMembers))
		{
			const std::lock_guard<std::mutex> lock(_mtxUsers);
			if (!_users.contains(member))
				throw ServerException("User " + member + " does not exist");
		}

		// insert userset's ID to each owner's owned usersets set
		for (const auto& owner : owners)
		{
			const std::lock_guard<std::mutex> lock(_mtxUsers);
			_users.at(owner).insert(setID);
		}

		// register shard IDs for all members
		{
			const std::lock_guard<std::mutex> lock(_mtxShardIDs);
			auto& usersetShardsEntry = _usersetShardIDs[setID];
			for (const auto& member : utils::views::join(owners, regMembers))
			{
				// generate unique, non-zero shard ID for this userset
				auto shardID = _shardsDist([](const auto& x) { return !x; }); // if !x, then x is invalid

				// register shard ID
				usersetShardsEntry.insert(shardID);
				_shardIDs.insert(std::make_pair(
					std::make_tuple(member, setID),
					shardID
				));
			}
		}

		return setID;
	}

	utils::HashSet<UserSetID> ShortTermServerStorage::get_usersets(const std::string& owner)
	{
		const std::lock_guard<std::mutex> lock(_mtxUsers);
		const auto it = _users.find(owner);
		if (it == _users.end())
			throw ServerException("User " + owner + "doesn't exist");
		return it->second;
	}

	bool ShortTermServerStorage::user_owns_userset(const std::string& user, const UserSetID& userset)
	{
		const std::lock_guard<std::mutex> lock(_mtxUsers);
		const auto it = _users.find(user);
		if (it == _users.end())
			throw ServerException("User " + user + "doesn't exist");
		return it->second.contains(userset);
	}

	UserSetInfo ShortTermServerStorage::get_userset_info(const UserSetID& userset)
	{
		const std::lock_guard<std::mutex> lock(_mtxUsersets);
		const auto it = _usersets.find(userset);
		if (it == _usersets.end())
			throw ServerException("No userset with ID " + userset.to_string());
		return it->second;
	}

	PrivKeyShardID ShortTermServerStorage::get_shard_id(const std::string& user, const UserSetID& userset)
	{
		const std::lock_guard<std::mutex> lock(_mtxShardIDs);
		return _shardIDs.at(std::make_tuple(user, userset));
	}
}

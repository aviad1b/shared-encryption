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
		: _shardsDist(utils::Random<PrivKeyShardID>::get_range_dist(1, MAX_MEMBERS)) { }

	void ShortTermServerStorage::new_user(const std::string& username)
	{
		bool inserted = false;
		{
			const std::lock_guard<std::mutex> lock(_mtxUsers);
			inserted = _users.insert(std::make_pair(
				username, std::set<UserSetID>{}
			)).second;
		}
		if (!inserted)
			throw UserExistsException(username);
	}

	bool ShortTermServerStorage::user_exists(const std::string& username)
	{
		const std::lock_guard<std::mutex> lock(_mtxUsers);
		return _users.contains(username);
	}

	UserSetID ShortTermServerStorage::new_userset(utils::ranges::StringViewRange&& owners,
												  utils::ranges::StringViewRange&& regMembers,
												  member_count_t ownersThreshold,
												  member_count_t regMembersThreshold)
	{
		StoredUserSetInfo info{
			utils::to_ordered_set<std::string>(owners),
			utils::to_ordered_set<std::string>(regMembers),
			ownersThreshold,
			regMembersThreshold
		};
		StoredUserSetInfo* pInfo = &info;

		// lock users for entire function to prevent changes while working
		// (e.g. we don't want member to get removed after we already checked it exists)
		const std::lock_guard<std::mutex> usersLock(_mtxUsers);

		// check if all members exist
		for (const auto& member : utils::views::join(pInfo->owners, pInfo->reg_members))
			if (!_users.contains(member))
				throw UserNotFoundException(member);

		UserSetID setID;

		// generate set ID and insert new userset to map
		{
			const std::lock_guard<std::mutex> lock(_mtxUsersets);
			setID = UserSetID::generate(_usersets);

			// move info into map, change pInfo to point at map element
			auto it = _usersets.insert(std::make_pair(
				setID,
				std::move(*pInfo)
			)).first;
			pInfo = &it->second;
		}

		// insert userset's ID to each owner's owned usersets set
		for (const auto& owner : pInfo->owners)
			_users.at(owner).insert(setID);

		// register shard IDs for all members
		{
			const std::lock_guard<std::mutex> lock(_mtxShardIDs);
			auto& usersetShardsEntry = _usersetShardIDs[setID];
			for (const auto& member : utils::views::join(pInfo->owners, pInfo->reg_members))
			{
				// generate unique, non-zero shard ID for this userset
				auto shardID = sample_shard_id(usersetShardsEntry);

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

	std::vector<UserSetID> ShortTermServerStorage::get_usersets(const std::string& owner)
	{
		const std::lock_guard<std::mutex> lock(_mtxUsers);
		const auto it = _users.find(owner);
		if (it == _users.end())
			throw UserNotFoundException(owner);
		return std::vector<UserSetID>(it->second.begin(), it->second.end());
	}

	bool ShortTermServerStorage::user_owns_userset(const std::string& user, const UserSetID& userset)
	{
		const std::lock_guard<std::mutex> lock(_mtxUsers);
		const auto it = _users.find(user);
		if (it == _users.end())
			throw UserNotFoundException(user);
		return it->second.contains(userset);
	}

	UserSetInfo ShortTermServerStorage::get_userset_info(const UserSetID& userset)
	{
		const std::lock_guard<std::mutex> lock(_mtxUsersets);
		const auto it = _usersets.find(userset);
		if (it == _usersets.end())
			throw UserSetNotFoundException(userset);
		return UserSetInfo{
			.owners = std::vector<std::string>(it->second.owners.begin(), it->second.owners.end()),
			.reg_members = std::vector<std::string>(it->second.reg_members.begin(), it->second.reg_members.end()),
			.owners_threshold = it->second.owners_threshold,
			.reg_members_threshold = it->second.reg_members_threshold
		};
	}

	PrivKeyShardID ShortTermServerStorage::get_shard_id(const std::string& user, const UserSetID& userset)
	{
		const std::lock_guard<std::mutex> lock(_mtxShardIDs);
		return _shardIDs.at(std::make_tuple(user, userset));
	}
}

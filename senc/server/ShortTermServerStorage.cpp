/*********************************************************************
 * \file   ShortTermServerStorage.cpp
 * \brief  Implementation of ShortTermServerStorage class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ShortTermServerStorage.hpp"

namespace senc::server
{
	OperationID ShortTermServerStorage::new_operation(const std::string& requester)
	{
		const std::lock_guard<std::mutex> lock(_mtxOperations);
		auto opid = OperationID::generate([this](const auto& x) { return _operations.contains(x); });
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
		const std::lock_guard<std::mutex> lock(_mtxUsersets);
		auto id = UserSetID::generate([this](const auto& x) { return _usersets.contains(x); });
		_usersets.insert(std::make_pair(
			id,
			UserSetInfo{
				owners,
				regMembers,
				ownersThreshold,
				regMembersThreshold
			}
		));
		return id;
	}
}

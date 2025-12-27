/*********************************************************************
 * \file   DecryptionsManager.cpp
 * \brief  Implementation of DecryptionsManager class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "DecryptionsManager.hpp"

#include "ServerException.hpp"

namespace senc::server
{
	bool DecryptionsManager::PrepareRecord::has_enough_members() const
	{
		return owners_found.size() >= required_owners &&
			reg_members_found.size() >= required_reg_members;
	}

	bool DecryptionsManager::CollectedRecord::has_enough_parts() const
	{
		return parts2.size() >= required_owners &&
			parts1.size() >= required_reg_members;
	}

	OperationID DecryptionsManager::new_operation()
	{
		const std::unique_lock<std::mutex> lock(_mtxAllOpIDs);
		auto opid = OperationID::generate(_allOpIDs);
		_allOpIDs.insert(opid);
		return opid;
	}

	void DecryptionsManager::prepare_operation(const OperationID& opid,
											   const std::string& requester,
											   const UserSetID& usersetID,
											   Ciphertext&& ciphertext,
											   member_count_t requiredOwners,
											   member_count_t requiredRegMembers)
	{
		const std::unique_lock<std::mutex> lock(_mtxPrep);
		_prep.emplace(opid, PrepareRecord{
			requester,
			usersetID,
			std::move(ciphertext),
			requiredOwners,
			requiredRegMembers
		});
	}

	std::pair<std::optional<DecryptionsManager::PrepareRecord>, bool>
		DecryptionsManager::register_participant(const OperationID& opid,
												 const std::string& username,
												 bool isOwner)
	{
		std::optional<PrepareRecord> res;

		// register participant
		const std::unique_lock<std::mutex> lockPrep(_mtxPrep);
		const auto it = _prep.find(opid);
		if (it == _prep.end())
		{
			if (_allOpIDs.contains(opid))
				return { res, false }; // operation ID is valid, already has enough users (user isn't required)
			else
				throw ServerException("No operation with ID " + opid.to_string()); // no such operation
		}

		// push participant to matching vector:
		// - if owner, try pushing into owners vec, if full then try pushing into non-owners vec
		// - if non-owner, try pushing into non-owners vec
		// - return false if failed (member isn't needed)
		if (isOwner && it->second.owners_found.size() < it->second.required_owners)
			it->second.owners_found.insert(username);
		else if (it->second.reg_members_found.size() < it->second.required_reg_members)
			it->second.reg_members_found.insert(username);
		else return { res, false }; // operation ID is valid, already has enough members

		// if has enough members, move from prepare stage to collect stage
		if (it->second.has_enough_members())
		{
			const std::unique_lock<std::mutex> lockColl(_mtxCollected);
			_collected.emplace(opid, CollectedRecord{
				it->second.requester,
				it->second.userset_id,
				it->second.required_owners,
				it->second.required_reg_members
			});
			res.emplace(std::move(it->second));
			_prep.erase(it);
		}

		return { res, true };
	}
	
	std::optional<DecryptionsManager::CollectedRecord>
		DecryptionsManager::register_part(const OperationID& opid,
										  DecryptionPart&& part,
										  PrivKeyShardID&& shardID,
										  bool isOwner)
	{
		std::optional<CollectedRecord> res;

		// register parts
		const std::unique_lock<std::mutex> lock(_mtxCollected);
		auto it = _collected.find(opid);
		auto& parts = isOwner ? it->second.parts2 : it->second.parts1;
		auto& shardsIDs = isOwner ? it->second.shardsIDs2 : it->second.shardsIDs1;
		parts.push_back(std::move(part));
		shardsIDs.push_back(std::move(shardID));

		// if has enough parts, remove record and return collect record
		if (it->second.has_enough_parts())
		{
			res.emplace(std::move(it->second));
			_collected.erase(it);
		}

		return res;
	}

	const UserSetID DecryptionsManager::get_operation_userset(const OperationID& opid)
	{
		{
			std::unique_lock<std::mutex> lock(_mtxPrep);
			const auto itPrep = _prep.find(opid);
			if (itPrep != _prep.end())
				return itPrep->second.userset_id;
		}

		{
			std::unique_lock<std::mutex> lock(_mtxCollected);
			const auto itColl = _collected.find(opid);
			if (itColl != _collected.end())
				return itColl->second.userset_id;
		}

		throw ServerException("Operation with ID " + opid.to_string() + " not found or already finished");
	}
}

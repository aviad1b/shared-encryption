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

	OperationID DecryptionsManager::register_new_operation(const std::string& requester,
														   const UserSetID& usersetID,
														   Ciphertext&& ciphertext,
														   member_count_t requiredOwners,
														   member_count_t requiredRegMembers)
	{
		OperationID opid{};
		{
			const std::unique_lock<std::mutex> lock(_mtxAllOpIDs);
			opid = OperationID::generate(_allOpIDs);
			_allOpIDs.insert(opid);
		}
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
		return opid;
	}

	std::optional<DecryptionsManager::PrepareRecord>
		DecryptionsManager::register_participant(const OperationID& opid,
												 const std::string& username,
												 bool isOwner)
	{
		std::optional<PrepareRecord> res;

		// register participant
		const std::unique_lock<std::mutex> lockPrep(_mtxPrep);
		const auto it = _prep.find(opid);
		auto& record = it->second;
		if (it == _prep.end())
			throw ServerException("No operation with ID " + opid.to_string());
		if (isOwner)
			record.owners_found.insert(username);
		else
			record.reg_members_found.insert(username);

		// if has enough members, move from prepare stage to collect stage
		if (record.has_enough_members())
		{
			const std::unique_lock<std::mutex> lockColl(_mtxCollected);
			_collected.emplace(opid, CollectedRecord{
				record.requester,
				record.userset_id,
				record.required_owners,
				record.required_reg_members
			});
			res.emplace(std::move(record));
			_prep.erase(it);
		}

		return res;
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
		auto& record = it->second;
		auto& parts = isOwner ? record.parts2 : record.parts1;
		auto& shardsIDs = isOwner ? record.shardsIDs2 : record.shardsIDs1;
		parts.push_back(std::move(part));
		shardsIDs.push_back(std::move(shardID));

		// if has enough parts, remove record and return collect record
		if (record.has_enough_parts())
		{
			res.emplace(std::move(record));
			_collected.erase(it);
		}

		return res;
	}

	const UserSetID DecryptionsManager::get_operation_userset(const OperationID& opid)
	{
		{
			std::unique_lock<std::mutex> lockPrep(_mtxPrep);
			const auto itPrep = _prep.find(opid);
			if (itPrep != _prep.end())
				return itPrep->second.userset_id;
		}

		{
			std::unique_lock<std::mutex> lockColl(_mtxCollected);
			const auto itColl = _collected.find(opid);
			if (itColl != _collected.end())
				return itColl->second.userset_id;
		}

		throw ServerException("No operation with ID " + opid.to_string());
	}
}

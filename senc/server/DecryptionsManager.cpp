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
	void DecryptionsManager::register_operation(const OperationID& opid,
												const std::string& requester,
												Ciphertext&& ciphertext,
												member_count_t requiredOwners,
												member_count_t requiredRegMembers)
	{
		const std::unique_lock<std::mutex> lock(_mtxPrep);
		_prep.emplace(opid, PrepareRecord{
			std::move(ciphertext),
			requiredOwners,
			requiredRegMembers
		});
	}

	const Ciphertext& DecryptionsManager::get_ciphertext(const OperationID& opid)
	{
		const std::unique_lock<std::mutex> lock(_mtxPrep);
		const auto it = _prep.find(opid);
		if (it == _prep.end())
		{
			throw ServerException(
				"Failed to retrieve ciphertext for operation " + opid.to_string(),
				"Either ID is invalid, or operation isn't currently looking for users"
			);
		}
		return it->second.ciphertext;
	}
	
	std::optional<DecryptionsManager::CollectedRecord>
		DecryptionsManager::register_part(const OperationID& opid,
										  DecryptionPart&& part,
										  PrivKeyShardID&& shardID,
										  bool isOwner)
	{
		std::optional<CollectedRecord> res;

		// register parts
		const std::unique_lock<std::mutex> lockColl(_mtxCollected);
		auto collRecordIt = _collected.emplace(opid).first;
		auto& collRecord = collRecordIt->second;
		auto& parts = isOwner ? collRecord.parts2 : collRecord.parts1;
		auto& shardsIDs = isOwner ? collRecord.shardsIDs2 : collRecord.shardsIDs1;
		parts.push_back(std::move(part));
		shardsIDs.push_back(std::move(shardID));

		// if has enough parts, remove records and return collect record
		const std::unique_lock<std::mutex> lockPrep(_mtxPrep);
		auto prepRecordIt = _prep.find(opid);
		const auto& prepRecord = prepRecordIt->second;
		if (collRecord.parts1.size() >= prepRecord.required_reg_members &&
			collRecord.parts2.size() >= prepRecord.required_owners)
		{
			res.emplace(std::move(collRecord));
			_collected.erase(collRecordIt);
			_prep.erase(prepRecordIt);
		}

		return res;
	}
}

/*********************************************************************
 * \file   UpdateManager.cpp
 * \brief  Implementation of UpdateManager class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "UpdateManager.hpp"

namespace senc::server
{
	pkt::UpdateResponse UpdateManager::retrieve_updates(const std::string& username)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		const auto it = _updates.find(username);
		if (it == _updates.end())
			return {}; // no updates
		return std::move(_updates.extract(it).mapped());
	}

	void UpdateManager::register_reg_member(const std::string& username,
											const UserSetID& usersetID,
											const PubKey& regLayerPubKey,
											const PubKey& ownerLayerPubKey,
											PrivKeyShard&& privKeyShard)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		_updates[username].added_as_reg_member.emplace_back(
			usersetID,
			regLayerPubKey, ownerLayerPubKey,
			std::move(privKeyShard)
		);
	}

	void UpdateManager::register_owner(const std::string& username,
									   const UserSetID& usersetID,
									   const PubKey& regLayerPubKey,
									   const PubKey& ownerLayerPubKey,
									   PrivKeyShard&& regLayerPrivKeyShard,
									   PrivKeyShard&& ownerLayerPrivKeyShard)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		_updates[username].added_as_owner.emplace_back(
			usersetID,
			regLayerPubKey, ownerLayerPubKey,
			std::move(regLayerPrivKeyShard), std::move(ownerLayerPrivKeyShard)
		);
	}

	void UpdateManager::register_lookup(const std::string& username, const OperationID& opid)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		_updates[username].on_lookup.push_back(opid);
	}

	void UpdateManager::register_decryption_participating(const std::string& username,
														  const OperationID& opid,
														  const Ciphertext& ciphertext,
														  const std::vector<PrivKeyShardID>& shardsIDs)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		_updates[username].to_decrypt.emplace_back(
			opid, ciphertext, shardsIDs
		);
	}

	void UpdateManager::register_finished_decrpytion(const std::string& username,
													 const OperationID& opid,
													 std::vector<DecryptionPart>&& regLayerParts,
													 std::vector<DecryptionPart>&& ownerLayerParts,
													 std::vector<PrivKeyShardID>&& regLayerShardsIDs,
													 std::vector<PrivKeyShardID>&& ownerLayerShardsIDs)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		_updates[username].finished_decryptions.emplace_back(
			opid, std::move(regLayerParts), std::move(ownerLayerParts),
			std::move(regLayerShardsIDs), std::move(ownerLayerShardsIDs)
		);
	}
}

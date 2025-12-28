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
											const PubKey& pubKey1, const PubKey& pubKey2,
											PrivKeyShard&& privKeyShard)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		_updates[username].added_as_reg_member.emplace_back(
			usersetID,
			pubKey1, pubKey2,
			std::move(privKeyShard)
		);
	}

	void UpdateManager::register_owner(const std::string& username,
									   const UserSetID& usersetID,
									   const PubKey& pubKey1, const PubKey& pubKey2,
									   PrivKeyShard&& privKeyShard1, PrivKeyShard&& privKeyShard2)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		_updates[username].added_as_owner.emplace_back(
			usersetID,
			pubKey1, pubKey2,
			std::move(privKeyShard1), std::move(privKeyShard2)
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
													 std::vector<DecryptionPart>&& parts1,
													 std::vector<DecryptionPart>&& parts2,
													 std::vector<PrivKeyShardID>&& shardsIDs1,
													 std::vector<PrivKeyShardID>&& shardsIDs2)
	{
		const std::lock_guard<std::mutex> lock(_mtxUpdates);
		_updates[username].finished_decryptions.emplace_back(
			opid, std::move(parts1), std::move(parts2),
			std::move(shardsIDs1), std::move(shardsIDs2)
		);
	}
}

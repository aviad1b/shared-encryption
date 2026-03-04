/*********************************************************************
 * \file   ProfileRecord.cpp
 * \brief  Implementation of ProfileRecord class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "ProfileRecord.hpp"

namespace senc::clientapi::storage
{
	ProfileRecord::Self ProfileRecord::owner(UserSetID&& usersetID,
											 PubKey&& regPubKey,
											 PubKey&& ownerPubKey,
											 PrivKeyShard&& regPrivKeyShard,
											 PrivKeyShard&& ownerExternalPrivKeyShard,
											 PrivKeyShard&& ownerInternalPrivKeyShard)
	{
		return Self(
			std::move(usersetID),
			std::move(regPubKey),
			std::move(ownerPubKey),
			std::move(regPrivKeyShard),
			OwnerPrivKeyShards{
				std::move(ownerExternalPrivKeyShard),
				std::move(ownerInternalPrivKeyShard)
			}
		);
	}

	ProfileRecord::Self ProfileRecord::reg(UserSetID&& usersetID,
										   PubKey&& regPubKey,
										   PubKey&& ownerPubKey,
										   PrivKeyShard&& regPrivKeyShard)
	{
		return Self(
			std::move(usersetID),
			std::move(regPubKey),
			std::move(ownerPubKey),
			std::move(regPrivKeyShard),
			std::nullopt
		);
	}

	bool ProfileRecord::is_owner() const noexcept
	{
		return _ownerPrivKeyShards.has_value();
	}

	const UserSetID& ProfileRecord::userset_id() const noexcept
	{
		return _usersetID;
	}

	const PubKey& ProfileRecord::reg_pub_key() const noexcept
	{
		return _regPubKey;
	}

	const PubKey& ProfileRecord::owner_pub_key() const noexcept
	{
		return _ownerPubKey;
	}

	const PrivKeyShard& ProfileRecord::reg_priv_key_shard() const noexcept
	{
		return _regPrivKeyShard;
	}

	const PrivKeyShard& ProfileRecord::owner_external_priv_key_shard() const noexcept
	{
		return _ownerPrivKeyShards->external;
	}

	const PrivKeyShard& ProfileRecord::owner_internal_priv_key_shard() const noexcept
	{
		return _ownerPrivKeyShards->internal;
	}

	ProfileRecord::ProfileRecord(UserSetID&& usersetID,
								 PubKey&& regPubKey,
								 PubKey&& ownerPubKey,
								 PrivKeyShard&& regPrivKeyShard,
								 std::optional<OwnerPrivKeyShards>&& ownerPrivKeyShards)
		: _usersetID(std::move(usersetID)),
		  _regPubKey(std::move(regPubKey)),
		  _ownerPubKey(std::move(ownerPubKey)),
		  _regPrivKeyShard(std::move(regPrivKeyShard)),
		  _ownerPrivKeyShards(std::move(ownerPrivKeyShards)) { }
}

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
											 PrivKeyShard&& ownerPrivKeyShard)
	{
		return Self(
			std::move(usersetID),
			std::move(regPubKey),
			std::move(ownerPubKey),
			std::move(regPrivKeyShard),
			std::move(ownerPrivKeyShard)
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
		return _ownerPrivKeyShard.has_value();
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

	const PrivKeyShard& ProfileRecord::owner_priv_key_shard() const noexcept
	{
		return *_ownerPrivKeyShard;
	}

	ProfileRecord::ProfileRecord(UserSetID&& usersetID,
								 PubKey&& regLayerPubKey,
								 PubKey&& ownerLayerPubKey,
								 PrivKeyShard&& regLayerPrivKeyShard,
								 std::optional<PrivKeyShard>&& ownerLayerPrivKeyShard)
		: _usersetID(std::move(usersetID)),
		  _regPubKey(std::move(regLayerPubKey)),
		  _ownerPubKey(std::move(ownerLayerPubKey)),
		  _regPrivKeyShard(std::move(regLayerPrivKeyShard)),
		  _ownerPrivKeyShard(std::move(ownerLayerPrivKeyShard)) { }
}

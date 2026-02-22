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
													PubKey&& regLayerPubKey,
													PubKey&& ownerLayerPubKey,
													PrivKeyShard&& regLayerPrivKeyShard,
													PrivKeyShard&& ownerLayerPrivKeyShard)
	{
		return Self(
			std::move(usersetID),
			std::move(regLayerPubKey),
			std::move(ownerLayerPubKey),
			std::move(regLayerPrivKeyShard),
			std::move(ownerLayerPrivKeyShard)
		);
	}

	ProfileRecord::Self ProfileRecord::reg(UserSetID&& usersetID,
										   PubKey&& regLayerPubKey,
										   PubKey&& ownerLayerPubKey,
										   PrivKeyShard&& regLayerPrivKeyShard)
	{
		return Self(
			std::move(usersetID),
			std::move(regLayerPubKey),
			std::move(ownerLayerPubKey),
			std::move(regLayerPrivKeyShard),
			std::nullopt
		);
	}

	bool ProfileRecord::is_owner() const
	{
		return _ownerLayerPrivKeyShard.has_value();
	}

	const UserSetID& ProfileRecord::userset_id() const
	{
		return _usersetID;
	}

	const PubKey& ProfileRecord::reg_layer_pub_key() const
	{
		return _regLayerPubKey;
	}

	const PubKey& ProfileRecord::owner_layer_pub_key() const
	{
		return _ownerLayerPubKey;
	}

	const PrivKeyShard& ProfileRecord::reg_layer_priv_key_shard() const
	{
		return _regLayerPrivKeyShard;
	}

	const PrivKeyShard& ProfileRecord::owner_layer_priv_key_shard() const
	{
		return *_ownerLayerPrivKeyShard;
	}

	ProfileRecord::ProfileRecord(UserSetID&& usersetID,
								 PubKey&& regLayerPubKey,
								 PubKey&& ownerLayerPubKey,
								 PrivKeyShard&& regLayerPrivKeyShard,
								 std::optional<PrivKeyShard>&& ownerLayerPrivKeyShard)
		: _usersetID(std::move(usersetID)),
		  _regLayerPubKey(std::move(regLayerPubKey)),
		  _ownerLayerPubKey(std::move(ownerLayerPubKey)),
		  _regLayerPrivKeyShard(std::move(regLayerPrivKeyShard)),
		  _ownerLayerPrivKeyShard(std::move(ownerLayerPrivKeyShard)) { }
}

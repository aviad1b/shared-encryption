/*********************************************************************
 * \file   ProfileRecord.hpp
 * \brief  Header of ProfileRecord class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "../../common/aliases.hpp"
#include <optional>

namespace senc::clientapi::storage
{
	/**
	 * @class senc::clientapi::storage::ProfileRecord
	 * @brief Represents a stored client record (info about a userset).
	 */
	class ProfileRecord
	{
	public:
		using Self = ProfileRecord;

		/**
		 * @brief Copy constructor of profile record.
		 */
		ProfileRecord(const Self&) = default;

		/**
		 * @brief Copy assignment operator of profile record.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Constructs a new owner profile record.
		 * @param usersetID Userset ID (moved).
		 * @param regLayerPubKey Public key of non-owner layer (moved).
		 * @param ownerLayerPubKey Public key of owner layer (moved).
		 * @param regLayerPrivKeyShard Private key shard of non-owner layer (moved).
		 * @param ownerLayerPrivKeyShard Private key shard of owner layer (moved).
		 * @return Constructed profile record.
		 */
		static Self owner(UserSetID&& usersetID,
						  PubKey&& regLayerPubKey,
						  PubKey&& ownerLayerPubKey,
						  PrivKeyShard&& regLayerPrivKeyShard,
						  PrivKeyShard&& ownerLayerPrivKeyShard);

		/**
		 * @brief Constructs a new non-owner profile record.
		 * @param usersetID Userset ID (moved).
		 * @param regLayerPubKey Public key of non-owner layer (moved).
		 * @param ownerLayerPubKey Public key of owner layer (moved).
		 * @param regLayerPrivKeyShard Private key shard of non-owner layer (moved).
		 * @return Constructed profile record.
		 */
		static Self reg(UserSetID&& usersetID,
						  PubKey&& regLayerPubKey,
						  PubKey&& ownerLayerPubKey,
						  PrivKeyShard&& regLayerPrivKeyShard);

		/**
		 * @brief Checks if this is an owner profile record.
		 * @return `true` if owner profile record, `false` if non-owner profile record.
		 */
		bool is_owner() const;

		/**
		 * @brief Gets userset ID.
		 * @return Userset ID.
		 */
		const UserSetID& userset_id() const;

		/**
		 * @brief Gets public key of non-owner layer.
		 * @return Public key of non-owner layer.
		 */
		const PubKey& reg_layer_pub_key() const;

		/**
		 * @brief Gets public key of owner layer.
		 * @return Public key of owner layer.
		 */
		const PubKey& owner_layer_pub_key() const;

		/**
		 * @brief Gets private key shard of non-owner layer.
		 * @return Private key shard of non-owner layer.
		 */
		const PrivKeyShard& reg_layer_priv_key_shard() const;
		
		/**
		 * @brief Gets private key shard of owner layer.
		 * @note Requires the profile record to be an owner profile record.
		 *		 Calling this method on a non-owner record is considered undefined behaviour.
		 * @return Private key shard of owner layer.
		 */
		const PrivKeyShard& owner_layer_priv_key_shard() const;

	private:
		UserSetID _usersetID;
		PubKey _regLayerPubKey;
		PubKey _ownerLayerPubKey;
		PrivKeyShard _regLayerPrivKeyShard;
		std::optional<PrivKeyShard> _ownerLayerPrivKeyShard;

		/**
		 * @brief Constructs a client profile record from moved fields.
		 * @param usersetID Userset ID (moved).
		 * @param regLayerPubKey Public key of non-owner layer (moved).
		 * @param ownerLayerPubKey Public key of owner layer (moved).
		 * @param regLayerPrivKeyShard Private key shard of non-owner layer (moved).
		 * @param ownerLayerPrivKeyShard Optional private key shard of owner layer (moved).
		 */
		ProfileRecord(UserSetID&& usersetID,
					  PubKey&& regLayerPubKey,
					  PubKey&& ownerLayerPubKey,
					  PrivKeyShard&& regLayerPrivKeyShard,
					  std::optional<PrivKeyShard>&& ownerLayerPrivKeyShard);
	};
}

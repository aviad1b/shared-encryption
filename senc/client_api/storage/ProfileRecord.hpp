/*********************************************************************
 * \file   ProfileRecord.hpp
 * \brief  Header of ProfileRecord class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "../../common/KeyEvolver.hpp"
#include "../../common/aliases.hpp"
#include <optional>

namespace senc::clientapi::storage
{
	/**
	 * @class senc::clientapi::storage::ProfileRecord
	 * @brief Represents a stored client record (info about a userset).
	 * @note For compatibility with storage reference wrappers, should not
	 *       implement any setters (these are replaced with transformation methods).
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
		 * @param nextEvolutionOffset Offset for next key evolution (moved).
		 * @param regPubKey Public key of non-owner layer (moved).
		 * @param ownerPubKey Public key of owner layer (moved).
		 * @param regExternalPrivKeyShard External private key shard of non-owner layer (moved).
		 * @param regInternalPrivKeyShard Internal private key shard of non-owner layer (moved).
		 * @param ownerExternalPrivKeyShard External private key shard of owner layer (moved).
		 * @param ownerInternalPrivKeyShard Internal private key shard of owner layer (moved).
		 * @return Constructed profile record.
		 */
		static Self owner(UserSetID&& usersetID,
						  utils::BigInt&& nextEvolutionOffset,
						  PubKey&& regPubKey,
						  PubKey&& ownerPubKey,
						  PrivKeyShard&& regExternalPrivKeyShard,
						  PrivKeyShard&& regInternalPrivKeyShard,
						  PrivKeyShard&& ownerExternalPrivKeyShard,
						  PrivKeyShard&& ownerInternalPrivKeyShard);

		/**
		 * @brief Constructs a new non-owner profile record.
		 * @param usersetID Userset ID (moved).
		 * @param nextEvolutionOffset Offset for next key evolution (moved).
		 * @param regPubKey Public key of non-owner layer (moved).
		 * @param ownerPubKey Public key of owner layer (moved).
		 * @param regExternalPrivKeyShard External private key shard of non-owner layer (moved).
		 * @return Constructed profile record.
		 */
		static Self reg(UserSetID&& usersetID,
						utils::BigInt&& nextEvolutionOffset,
						PubKey&& regPubKey,
						PubKey&& ownerPubKey,
						PrivKeyShard&& regExternalPrivKeyShard);

		/**
		 * @brief Checks if this is an owner profile record.
		 * @return `true` if owner profile record, `false` if non-owner profile record.
		 */
		bool is_owner() const noexcept;

		/**
		 * @brief Gets userset ID.
		 * @return Userset ID.
		 */
		const UserSetID& userset_id() const noexcept;

		/**
		 * @brief Gets offset for next key evolution.
		 * @return Offset for next key evolution.
		 */
		const utils::BigInt& next_evolution_offset() const noexcept;

		/**
		 * @brief Gets public key of non-owner layer.
		 * @return Public key of non-owner layer.
		 */
		const PubKey& reg_pub_key() const noexcept;

		/**
		 * @brief Gets public key of owner layer.
		 * @return Public key of owner layer.
		 */
		const PubKey& owner_pub_key() const noexcept;

		/**
		 * @brief Gets external private key shard of non-owner layer.
		 * @return Private key shard of non-owner layer.
		 */
		const PrivKeyShard& reg_external_priv_key_shard() const noexcept;

		/**
		 * @brief Gets internal private key shard of non-owner layer.
		 * @note Requires the profile record to be an owner profile record.
		 *		 Calling this method on a non-owner record is considered undefined behaviour.
		 * @return Internal private key shard of owner layer.
		 */
		const PrivKeyShard& reg_internal_priv_key_shard() const noexcept;
		
		/**
		 * @brief Gets external private key shard of owner layer.
		 * @note Requires the profile record to be an owner profile record.
		 *		 Calling this method on a non-owner record is considered undefined behaviour.
		 * @return External private key shard of owner layer.
		 */
		const PrivKeyShard& owner_external_priv_key_shard() const noexcept;

		/**
		 * @brief Gets internal private key shard of owner layer.
		 * @note Requires the profile record to be an owner profile record.
		 *		 Calling this method on a non-owner record is considered undefined behaviour.
		 * @return Internal private key shard of owner layer.
		 */
		const PrivKeyShard& owner_internal_priv_key_shard() const noexcept;

		/**
		 * @brief Moves profile record into new instance with different next evolution offset.
		 * @param offset New offset value (moved).
		 * @return New profile record instance.
		 */
		Self transform_next_evolution_offset(utils::BigInt&& offset);

		/**
		 * @brief Applies key evolution on profile record and moves into new instance.
		 * @param evolve Key evolver to use for evolution.
		 * @return New profile record instance.
		 */
		Self transform_evolve(KeyEvolver& evolve);

	private:
		struct OwnerPrivKeyShards { PrivKeyShard regInternal, ownerExternal, ownerInternal; };
		UserSetID _usersetID;
		utils::BigInt _nextEvolutionOffset;
		PubKey _regPubKey;
		PubKey _ownerPubKey;
		PrivKeyShard _regExternalPrivKeyShard;
		std::optional<OwnerPrivKeyShards> _ownerPrivKeyShards;

		/**
		 * @brief Constructs a client profile record from moved fields.
		 * @param usersetID Userset ID (moved).
		 * @param nextEvolutionOffset Offset for next key evolution (moved).
		 * @param regPubKey Public key of non-owner layer (moved).
		 * @param ownerPubKey Public key of owner layer (moved).
		 * @param regExternalPrivKeyShard External private key shard of non-owner (moved).
		 * @param ownerPrivKeyShards Optional private key shards of owner (moved).
		 */
		ProfileRecord(UserSetID&& usersetID,
					  utils::BigInt&& nextEvolutionOffset,
					  PubKey&& regPubKey,
					  PubKey&& ownerPubKey,
					  PrivKeyShard&& regExternalPrivKeyShard,
					  std::optional<OwnerPrivKeyShards>&& ownerPrivKeyShards);
	};
}

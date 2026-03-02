/*********************************************************************
 * \file   KeyedPacketHandlerSyncData.hpp
 * \brief  Header of `KeyedPacketHandlerSyncData` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "IPacketHandlerSyncData.hpp"
#include "../utils/enc/general.hpp"

namespace senc
{
	/**
	 * @class senc::KeyedPacketHandlerSyncData
	 * @brief Sync data implementation for encrypted packet handlers.
	 * @tparam Key Key type.
	 */
	template <std::equality_comparable Key>
	class KeyedPacketHandlerSyncData : public IPacketHandlerSyncData
	{
	public:
		using Self = KeyedPacketHandlerSyncData;

		/**
		 * @brief Constructs keyed packet handler data with default-constructed key.
		 * @note Requires `Key` to be default constructible.
		 */
		KeyedPacketHandlerSyncData() requires std::default_initializable<Key> = default;

		/**
		 * @brief Constructs keyed packet handler data from moved key.
		 * @note Requires `Key` to be move constructible.
		 * @param key Key (moved).
		 */
		KeyedPacketHandlerSyncData(Key&& key)
		requires std::move_constructible<Key>;

		/**
		 * @brief Move constructor of keyed packet handler data.
		 * @note Requires `Key` to be move constructible.
		 * @param other Keyed packet handler data to move-construct from.
		 */
		KeyedPacketHandlerSyncData(Self&& other)
		requires std::move_constructible<Key>;

		/**
		 * @brief Move assignment operator of keyed packet handler data.
		 * @note Requires `Key` to be move assignable.
		 * @param other Other keyed packet handler to move into this one.
		 */
		Self& operator=(Self&& other)
		requires std::is_move_assignable_v<Key>;

		bool validate_synchronization(const IPacketHandlerSyncData& other) const override;

		/**
		 * @brief Gets synchronized key.
		 * @return Key.
		 */
		const Key& get_key() const;

		/**
		 * @brief Sets stored synchronized key.
		 * @note Requires `Key` to be move-constructible.
		 * @param key New key instance (moved).
		 */
		void set_key(Key&& key)
		requires std::is_move_assignable_v<Key>;

	private:
		Key _key;
	};
}

#include "KeyedPacketHandlerSyncData_impl.hpp"

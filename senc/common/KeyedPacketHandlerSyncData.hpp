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

		KeyedPacketHandlerSyncData() requires std::default_initializable<Key> = default;

		KeyedPacketHandlerSyncData(Key&& key)
		requires std::move_constructible<Key>;

		KeyedPacketHandlerSyncData(Self&& other)
		requires std::move_constructible<Key>;

		Self& operator=(Self&& other)
		requires std::is_move_assignable_v<Key>;

		bool validate_synchronization(const IPacketHandlerSyncData& other) const override;

		const Key& get_key() const;

		void set_key(Key&& key)
		requires std::is_move_assignable_v<Key>;

	private:
		Key _key;
	};
}

#include "KeyedPacketHandlerSyncData_impl.hpp"

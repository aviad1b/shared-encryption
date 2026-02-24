/*********************************************************************
 * \file   KeyedPacketHandlerSyncData_impl.hpp
 * \brief  Implementation of `KeyedPacketHandlerSyncData` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "KeyedPacketHandlerSyncData.hpp"

namespace senc
{
	template <std::equality_comparable Key>
	inline KeyedPacketHandlerSyncData<Key>::KeyedPacketHandlerSyncData(Key&& key)
	requires std::move_constructible<Key>
		: _key(std::move(key)) { }

	template <std::equality_comparable Key>
	inline KeyedPacketHandlerSyncData<Key>::KeyedPacketHandlerSyncData(Self&& other)
	requires std::move_constructible<Key>
		: _key(std::move(other._key)) { }

	template <std::equality_comparable Key>
	inline KeyedPacketHandlerSyncData<Key>::Self& KeyedPacketHandlerSyncData<Key>::operator=(Self&& other)
	requires std::is_move_assignable_v<Key>
	{
		this->_key = std::move(other._key);
	}

	template <std::equality_comparable Key>
	inline bool senc::KeyedPacketHandlerSyncData<Key>::validate_synchronization(const IPacketHandlerSyncData* other) const
	{
		// return false if other is not of encrypted handler
		const Self* other2 = dynamic_cast<const Self*>(other);
		if (!other2)
			return false;

		// check synchronized keys
		return (this->_key == other2->_key);
	}
}

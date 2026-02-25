/*********************************************************************
 * \file   PlainPacketHandlerSyncData.cpp
 * \brief  Implementation of `PlainPacketHandlerSyncData` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "PlainPacketHandlerSyncData.hpp"

namespace senc
{
	bool PlainPacketHandlerSyncData::validate_synchronization(const IPacketHandlerSyncData& other) const
	{
		// return true iff other is also non-encrypted sync data
		return (nullptr != dynamic_cast<const Self*>(&other));
	}
}

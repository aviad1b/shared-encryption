/*********************************************************************
 * \file   PlainPacketHandlerSyncData.hpp
 * \brief  Header of `PlainPacketHandlerSyncData` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "IPacketHandlerSyncData.hpp"

namespace senc
{
	/**
	 * @class senc::PlainPacketHandlerSyncData
	 * @brief Sync data implementation for non-encrypted packet handlers.
	 */
	class PlainPacketHandlerSyncData : public IPacketHandlerSyncData
	{
	public:
		using Self = PlainPacketHandlerSyncData;

		PlainPacketHandlerSyncData() = default;
		
		PlainPacketHandlerSyncData(Self&&) = default;

		bool validate_synchronization(const IPacketHandlerSyncData* other) const override;
	};
}

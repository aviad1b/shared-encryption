/*********************************************************************
 * \file   IPacketHandlerSyncData.hpp
 * \brief  Contains `IPacketHandlerSyncData` interface.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

namespace senc
{
	/**
	 * @interface senc::IPacketHandlerSyncData
	 * @brief Holds data of packet handler which is supposed to be synchronized between client and server.
	 */
	class IPacketHandlerSyncData
	{
	public:
		/**
		 * @brief Validates that another packet handler data is synchronized with this one.
		 * @return `true` if synchronized, otherwise `false`.
		 */
		virtual bool validate_synchronization(const IPacketHandlerSyncData& other) const = 0;
	};
}

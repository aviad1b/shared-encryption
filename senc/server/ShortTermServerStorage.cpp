/*********************************************************************
 * \file   ShortTermServerStorage.cpp
 * \brief  Implementation of ShortTermServerStorage class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ShortTermServerStorage.hpp"

namespace senc::server
{
	OperationID ShortTermServerStorage::new_operation(const std::string& requester)
	{
		OperationID opid = OperationID::generate();
		{
			const std::lock_guard<std::mutex> lock(_mtxOperations);

			// make sure operation ID is unique
			while (_operations.contains(opid))
				opid = OperationID::generate();

			// register operation
			_operations.insert(std::make_pair(opid, requester));
		}
		return opid;
	}
}

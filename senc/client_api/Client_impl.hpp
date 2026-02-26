/*********************************************************************
 * \file   Client_impl.hpp
 * \brief  Implementation of `Client` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "Client.hpp"

namespace senc::clientapi
{
	template <utils::IPType IP>
	inline Client<IP>::Client(const IP& serverIP, utils::Port& serverPort,
							  std::function<Schema()> schemaFactory,
							  ClientPacketHandlerFactory packetHandlerFactory,
							  std::function<void(const OperationID&, const utils::Buffer&)> decryptFinishedCallback)
		: _decryptFinishedCallback(decryptFinishedCallback),
		  _packetHandlerFactory(packetHandlerFactory),
		  _schema(schemaFactory()),
		  _sock(serverIP, serverPort) { }
}

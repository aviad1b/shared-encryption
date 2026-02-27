/*********************************************************************
 * \file   Client_impl.hpp
 * \brief  Implementation of `Client` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "Client.hpp"

#include "ClientException.hpp"

namespace senc::clientapi
{
	template <utils::IPType IP>
	inline Client<IP>::Client(const IP& serverIP, utils::Port& serverPort,
							  std::function<Schema()> schemaFactory,
							  ClientPacketHandlerFactory packetHandlerFactory,
							  std::function<void(const OperationID&, const utils::Buffer&)> decryptFinishedCallback)
		: _serverIP(serverIP), _serverPort(serverPort),
		  _decryptFinishedCallback(decryptFinishedCallback),
		  _packetHandlerFactory(packetHandlerFactory),
		  _schema(schemaFactory()),
		  _sock(serverIP, serverPort)
	{
		this->_packetHandler = _packetHandlerFactory(_sock);
	}

	template <utils::IPType IP>
	inline Client<IP>::~Client()
	{
		if (this->_packetHandler) // if still connected (packet handler not null)
			logout();
	}

	template <utils::IPType IP>
	inline void senc::clientapi::Client<IP>::ensure_connected()
	{
		if (this->_packetHandler) // if connected (packet handler not null)
			return; // nothing to do

		// if not connected (disconnected earlier) - reconnect
		this->_sock = Socket(_serverIP, _serverPort);
		this->_packetHandler = _packetHandlerFactory(_sock);
	}

	template <utils::IPType IP>
	template <typename Resp, typename Req>
	inline Resp senc::clientapi::Client<IP>::post(const Req& request)
	{
		_packetHandler->send_request(request);
		auto resp = _packetHandler->recv_response<Resp, pkt::ErrorResponse>();
		if (!resp)
			throw ClientException("Unexpected response received");
		if (std::holds_alternative<pkt::ErrorResponse>(*resp))
			throw ClientException(std::get<pkt::ErrorResponse>(*resp).msg);
		return std::get<Resp>(*resp);
	}
}

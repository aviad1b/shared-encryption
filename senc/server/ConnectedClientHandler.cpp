/*********************************************************************
 * \file   ConnectedClientHandler.cpp
 * \brief  Implementation of ConnectedClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ConnectedClientHandler.hpp"

namespace senc::server
{
	ConnectedClientHandler::ConnectedClientHandler(utils::Socket& sock,
												   const std::string& username,
												   IServerStorage& storage,
												   PacketReceiver& receiver,
												   PacketSender& sender)
		: _sock(sock), _username(username), _storage(storage),
		  _receiver(receiver), _sender(sender) { }

	void ConnectedClientHandler::loop()
	{
		Status status = Status::Connected;
		while (Status::Connected == status)
		{
			status = iteration();
		}
	}

	ConnectedClientHandler::Status ConnectedClientHandler::iteration()
	{
		auto req = _receiver.recv_request<
			pkt::LogoutRequest,
			pkt::MakeUserSetRequest,
			pkt::GetUserSetsRequest,
			pkt::GetMembersRequest,
			pkt::DecryptRequest,
			pkt::UpdateRequest,
			pkt::DecryptParticipateRequest,
			pkt::SendDecryptionPartRequest
		>(_sock);

		if (req.has_value())
			return std::visit(
				[this](const auto& r) { return handle_request(r); },
				*req
			);

		// if reached here, bad request
		_sender.send_response(_sock, pkt::ErrorResponse{ "Bad request" });
		return Status::Connected;
	}
}

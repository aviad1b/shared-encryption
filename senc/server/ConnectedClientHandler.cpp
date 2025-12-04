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
												   Schema& schema,
												   IServerStorage& storage,
												   PacketReceiver& receiver,
												   PacketSender& sender,
												   UpdateManager& updateManager,
												   DecryptionsManager& decryptionsManager)
		: _sock(sock), _username(username),
		  _schema(schema), _storage(storage),
		  _receiver(receiver), _sender(sender),
		  _updateManager(updateManager), _decryptionsManager(decryptionsManager) { }

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
				[this](auto& r) { return handle_request(r); },
				*req
			);

		// if reached here, bad request
		_sender.send_response(_sock, pkt::ErrorResponse{ "Bad request" });
		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::LogoutRequest& request)
	{
		(void)request;
		_sender.send_response(_sock, pkt::LogoutResponse{});
		return Status::Disconnected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::MakeUserSetRequest& request)
	{
		pkt::MakeUserSetResponse response{};
		try
		{
			response = make_userset(
				_username,
				request.owners, request.reg_members,
				request.owners_threshold, request.reg_members_threshold
			);
		}
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to create userset: ") + e.what()
			});
			return Status::Connected;
		}

		_sender.send_response(_sock, response);
		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::GetUserSetsRequest& request)
	{
		(void)request;

		utils::HashSet<UserSetID> usersets;
		try { usersets = _storage.get_usersets(_username); }
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to fetch usersets: ") + e.what()
			});
			return Status::Connected;
		}

		pkt::GetUserSetsResponse response{};
		response.user_sets_ids.insert(
			response.user_sets_ids.end(),
			usersets.begin(), usersets.end()
		);
		// TODO: Sort?

		_sender.send_response(_sock, response);

		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::GetMembersRequest& request)
	{
		UserSetInfo info{};
		try { info = _storage.get_userset_info(request.user_set_id); }
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to fetch userset members: ") + e.what()
			});
			return Status::Connected;
		}

		pkt::GetMembersResponse response{};
		response.owners.insert(
			response.owners.end(),
			info.owners.begin(), info.owners.end()
		);
		response.reg_members.insert(
			response.reg_members.end(),
			info.reg_members.begin(),
			info.reg_members.end()
		);
		// TODO: Sort?

		_sender.send_response(_sock, response);

		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::DecryptRequest& request)
	{
		OperationID opid{};

		try { opid = initiate_decryption(request.user_set_id, std::move(request.ciphertext)); }
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to initiate decryption operation: ") + e.what()
			});
			return Status::Connected;
		}

		_sender.send_response(_sock, pkt::DecryptResponse{ opid });
		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::UpdateRequest& request)
	{
		(void)request;
		return Status::Connected; // TODO: Implement
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::DecryptParticipateRequest& request)
	{
		(void)request;
		return Status();
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::SendDecryptionPartRequest& request)
	{
		(void)request;
		return Status();
	}
}

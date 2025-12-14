/*********************************************************************
 * \file   ConnectingClientHandler.cpp
 * \brief  Implementation of ConnectingClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ConnectingClientHandler.hpp"

namespace senc::server
{
	ConnectingClientHandler::ConnectingClientHandler(utils::Socket& sock,
													 IServerStorage& storage,
													 PacketReceiver& receiver,
													 PacketSender& sender)
		: _sock(sock), _storage(storage), _receiver(receiver), _sender(sender) { }

	std::tuple<bool, std::string> ConnectingClientHandler::connect_client()
	{
		// check client's protocol version
		auto protocolVersion = _sock.recv_connected_primitive<std::uint8_t>();
		if (protocolVersion != pkt::PROTOCOL_VERSION)
		{
			_sock.send_connected_primitive(false); // bad protocol version
			return { false, "" }; // user disconnected
		}
		_sock.send_connected_primitive(true); // good protocol version

		// run login/signup loop
		std::string username;
		Status status{};
		do
		{
			// connection request: Should be signup, login or logout (to disconnect)
			auto connReq = _receiver.recv_request<
				pkt::SignupRequest,
				pkt::LoginRequest,
				pkt::LogoutRequest
			>(_sock);
			while (!connReq.has_value())
			{
				_sender.send_response(_sock, pkt::ErrorResponse{ "Bad request" });
				connReq = _receiver.recv_request<
					pkt::SignupRequest,
					pkt::LoginRequest,
					pkt::LogoutRequest
				>(_sock);
			}

			// call fitting client_loop implementation based on connection request
			std::tie(status, username) = std::visit(
				[this](const auto& req) { return handle_request(req); },
				*connReq
			);
		} while (Status::Error == status);

		return { Status::Connected == status, username };
	}

	std::tuple<ConnectingClientHandler::Status, std::string>
		ConnectingClientHandler::handle_request(const pkt::SignupRequest signup)
	{
		try { _storage.new_user(signup.username); }
		catch (const UserExistsException& e)
		{
			_sender.send_response(_sock, pkt::SignupResponse{ pkt::SignupResponse::Status::UsernameTaken });
			return { Status::Error, "" };
		}
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{ e.what() });
			return { Status::Error, "" };
		}

		_sender.send_response(_sock, pkt::SignupResponse{ pkt::SignupResponse::Status::Success });
		return { Status::Connected, signup.username };
	}

	std::tuple<ConnectingClientHandler::Status, std::string>
		ConnectingClientHandler::handle_request(const pkt::LoginRequest login)
	{
		if (!_storage.user_exists(login.username))
		{
			_sender.send_response(_sock, pkt::LoginResponse{ pkt::LoginResponse::Status::BadUsername });
			return { Status::Error, "" };
		}

		_sender.send_response(_sock, pkt::LoginResponse{ pkt::LoginResponse::Status::Success });
		return { Status::Connected, login.username }; // handled, connected
	}

	std::tuple<ConnectingClientHandler::Status, std::string>
		ConnectingClientHandler::handle_request(const pkt::LogoutRequest logout)
	{
		(void)logout;
		_sender.send_response(_sock, pkt::LogoutResponse{});
		return { Status::Disconnected, "" };
	}
}

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
	ConnectingClientHandler::ConnectingClientHandler(ILogger& logger,
													 PacketHandler& packetHandler,
													 IServerStorage& storage)
		: _logger(logger), _packetHandler(packetHandler), _storage(storage) { }

	std::tuple<bool, std::string> ConnectingClientHandler::connect_client()
	{
		// run login/signup loop
		std::string username;
		Status status{};
		do
		{
			// connection request: Should be signup, login or logout (to disconnect)
			auto connReq = _packetHandler.recv_request<
				pkt::SignupRequest,
				pkt::LoginRequest,
				pkt::LogoutRequest
			>();
			while (!connReq.has_value())
			{
				_packetHandler.send_response(pkt::ErrorResponse{ "Bad request" });
				connReq = _packetHandler.recv_request<
					pkt::SignupRequest,
					pkt::LoginRequest,
					pkt::LogoutRequest
				>();
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
		try { _storage.new_user(signup.username, signup.password); }
		catch (const UserExistsException&)
		{
			_packetHandler.send_response(pkt::SignupResponse{ pkt::SignupResponse::Status::UsernameTaken });
			return { Status::Error, "" };
		}
		catch (const ServerException& e)
		{
			_packetHandler.send_response(pkt::ErrorResponse{ e.what() });
			return { Status::Error, "" };
		}

		_packetHandler.send_response(pkt::SignupResponse{ pkt::SignupResponse::Status::Success });
		return { Status::Connected, signup.username };
	}

	std::tuple<ConnectingClientHandler::Status, std::string>
		ConnectingClientHandler::handle_request(const pkt::LoginRequest login)
	{
		if (!_storage.user_has_password(login.username, login.password))
		{
			_packetHandler.send_response(pkt::LoginResponse{ pkt::LoginResponse::Status::BadLogin });
			return { Status::Error, "" };
		}

		_packetHandler.send_response(pkt::LoginResponse{ pkt::LoginResponse::Status::Success });
		return { Status::Connected, login.username }; // handled, connected
	}

	std::tuple<ConnectingClientHandler::Status, std::string>
		ConnectingClientHandler::handle_request(const pkt::LogoutRequest logout)
	{
		(void)logout;
		_packetHandler.send_response(pkt::LogoutResponse{});
		return { Status::Disconnected, "" };
	}
}

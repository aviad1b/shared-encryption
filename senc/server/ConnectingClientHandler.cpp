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
													 PacketHandler& packetHandler)
		: _sock(sock), _storage(storage), _packetHandler(packetHandler) { }

	std::tuple<bool, std::string> ConnectingClientHandler::connect_client()
	{
		const bool validConn = _packetHandler.establish_connection_server_side(_sock).first;
		if (!validConn)
			return { false, "" };

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
			>(_sock);
			while (!connReq.has_value())
			{
				_packetHandler.send_response(_sock, pkt::ErrorResponse{ "Bad request" });
				connReq = _packetHandler.recv_request<
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
		try { _storage.new_user(signup.username, signup.password); }
		catch (const UserExistsException&)
		{
			_packetHandler.send_response(_sock, pkt::SignupResponse{ pkt::SignupResponse::Status::UsernameTaken });
			return { Status::Error, "" };
		}
		catch (const ServerException& e)
		{
			_packetHandler.send_response(_sock, pkt::ErrorResponse{ e.what() });
			return { Status::Error, "" };
		}

		_packetHandler.send_response(_sock, pkt::SignupResponse{ pkt::SignupResponse::Status::Success });
		return { Status::Connected, signup.username };
	}

	std::tuple<ConnectingClientHandler::Status, std::string>
		ConnectingClientHandler::handle_request(const pkt::LoginRequest login)
	{
		if (!_storage.user_has_password(login.username, login.password))
		{
			_packetHandler.send_response(_sock, pkt::LoginResponse{ pkt::LoginResponse::Status::BadLogin });
			return { Status::Error, "" };
		}

		_packetHandler.send_response(_sock, pkt::LoginResponse{ pkt::LoginResponse::Status::Success });
		return { Status::Connected, login.username }; // handled, connected
	}

	std::tuple<ConnectingClientHandler::Status, std::string>
		ConnectingClientHandler::handle_request(const pkt::LogoutRequest logout)
	{
		(void)logout;
		_packetHandler.send_response(_sock, pkt::LogoutResponse{});
		return { Status::Disconnected, "" };
	}
}

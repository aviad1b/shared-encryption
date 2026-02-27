/*********************************************************************
 * \file   Client_impl.hpp
 * \brief  Implementation of `Client` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "Client.hpp"

#include "ClientException.hpp"
#include "ClientUtils.hpp"

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
	inline void Client<IP>::signup(const std::string& username, const std::string& password)
	{
		ensure_connected();

		auto resp = this->post<pkt::SignupResponse>(pkt::SignupRequest{
			username, password
		});

		if (resp.status == pkt::SignupResponse::Status::UsernameTaken)
			throw ClientException("Signup failed", "Username already taken");
		if (resp.status != pkt::SignupResponse::Status::Success)
			throw ClientException("Signup failed", "Unknown error");

		this->load_profile(username, password);
	}

	template <utils::IPType IP>
	inline void Client<IP>::login(const std::string& username, const std::string& password)
	{
		ensure_connected();

		auto resp = this->post<pkt::LoginResponse>(pkt::LoginRequest{
			username, password
		});

		if (resp.status == pkt::LoginResponse::Status::BadLogin)
			throw ClientException("Login failed", "Bad username or password");
		if (resp.status != pkt::LoginResponse::Status::Success)
			throw ClientException("Login failed", "Unknown error");

		this->load_profile(username, password);
	}

	template <utils::IPType IP>
	inline void Client<IP>::logout()
	{
		this->post<pkt::LogoutResponse>(pkt::LogoutRequest{});
		this->_packetHandler.reset();
		this->_sock.close();
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
	inline void Client<IP>::load_profile(const std::string& username, const std::string& password)
	{
		_storage.emplace(
			ClientUtils::locate_user_profile_file(username, password),
			username, password
		);
	}

	template <utils::IPType IP>
	inline void Client<IP>::unload_profile()
	{
		_storage.reset();
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

/*********************************************************************
 * \file   EncryptedPacketHandler.hpp
 * \brief  Header of EncryptedPacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "PacketHandler.hpp"

namespace senc
{
	class EncryptedPacketHandler : public PacketHandler
	{
	public:
		using Self = EncryptedPacketHandler;
		using Base = PacketHandler;

		void send_connection_request(utils::Socket& sock) override;
		bool recv_connection_request(utils::Socket& sock) override;

		void send_connection_response(utils::Socket& sock, bool isConnectionValid) override;
		bool recv_connection_response(utils::Socket& sock) override;

		void send_response_data(utils::Socket& sock, const pkt::ErrorResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::SignupRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::SignupRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::SignupResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::SignupResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::LoginRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::LoginRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::LoginResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::LoginResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::LogoutRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::LogoutRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::LogoutResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::LogoutResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::MakeUserSetRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::MakeUserSetRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::MakeUserSetResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::MakeUserSetResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::GetUserSetsRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::GetUserSetsRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::GetUserSetsResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::GetUserSetsResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::GetMembersRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::GetMembersRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::GetMembersResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::GetMembersResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::DecryptRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::DecryptRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::DecryptResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::DecryptResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::UpdateRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::UpdateRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::UpdateResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::UpdateResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::DecryptParticipateRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::DecryptParticipateResponse& out) override;

		void send_request_data(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet) override;
		void recv_request_data(utils::Socket& sock, pkt::SendDecryptionPartRequest& out) override;

		void send_response_data(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet) override;
		void recv_response_data(utils::Socket& sock, pkt::SendDecryptionPartResponse& out) override;
	};
}

/*********************************************************************
 * \file   EncryptedPacketSender.hpp
 * \brief  Header of EncryptedPacketSender class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "PacketSender.hpp"

namespace senc
{
	/**
	 * @class senc::EncryptedPacketSender
	 * @brief Encrypted implementation of `senc::PacketSender`.
	 */
	class EncryptedPacketSender : public PacketSender
	{
	public:
		using Self = EncryptedPacketSender;
		using Base = PacketSender;

		void send_connection_request(utils::Socket& sock) override;

		void send_connection_response(utils::Socket& sock, bool isConnectionValid) override;

		void send_response_data(utils::Socket& sock, const pkt::ErrorResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::SignupRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::SignupResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::LoginRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::LoginResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::LogoutRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::LogoutResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::MakeUserSetRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::MakeUserSetResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::GetUserSetsRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::GetUserSetsResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::GetMembersRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::GetMembersResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::DecryptRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::DecryptResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::UpdateRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::UpdateResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet) override;

		void send_request_data(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet) override;
		void send_response_data(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet) override;
	};
}

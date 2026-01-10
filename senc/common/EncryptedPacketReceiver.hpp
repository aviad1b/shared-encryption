/*********************************************************************
 * \file   EncryptedPacketReceiver.hpp
 * \brief  Header of EncryptedPacketReceiver class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "PacketReceiver.hpp"

namespace senc
{
	/**
	 * @class senc::EncryptedPacketReceiver
	 * @brief Encrypted implementation of `senc::PacketReceiver`.
	 */
	class EncryptedPacketReceiver : public PacketReceiver
	{
	public:
		using Self = EncryptedPacketReceiver;
		using Base = PacketReceiver;

		bool recv_connection_request(utils::Socket& sock) override;

		bool recv_connection_response(utils::Socket& sock) override;

		void recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::SignupRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::SignupResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::LoginRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::LoginResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::LogoutRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::LogoutResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::MakeUserSetRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::MakeUserSetResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::GetUserSetsRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::GetUserSetsResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::GetMembersRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::GetMembersResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::DecryptRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::DecryptResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::UpdateRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::UpdateResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::DecryptParticipateRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::DecryptParticipateResponse& out) override;

		void recv_request_data(utils::Socket& sock, pkt::SendDecryptionPartRequest& out) override;
		void recv_response_data(utils::Socket& sock, pkt::SendDecryptionPartResponse& out) override;
	};
}

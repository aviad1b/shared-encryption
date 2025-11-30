/*********************************************************************
 * \file   PacketReceiver.hpp
 * \brief  Contains abstract class PacketReceiver.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <optional>
#include "../utils/variants.hpp"
#include "../utils/Socket.hpp"
#include "packets.hpp"

namespace senc
{
	class PacketReceiver
	{
	public:
		using Self = PacketReceiver;

		virtual ~PacketReceiver() { }

		/**
		 * @brief Receives a request of one of the given types.
		 * @tparam Ts Potential packet types (structs).
		 * @return Received packet, or `std::nullopt` if was of wrong type.
		 * @note Assuming any valid packet was sent, `sock` is guarenteed to be clear after call.
		 */
		template <typename... Ts>
		inline std::optional<utils::VariantOrSingular<Ts...>> recv_request(utils::Socket& sock)
		{
			return recv_packet<PacketKind::Request, Ts...>(sock);
		}

		/**
		 * @brief Receives a response of one of the given types.
		 * @tparam Ts Potential packet types (structs).
		 * @return Received packet, or `std::nullopt` if was of wrong type.
		 * @note Assuming any valid packet was sent, `sock` is guarenteed to be clear after call.
		 */
		template <typename... Ts>
		inline std::optional<utils::VariantOrSingular<Ts...>> recv_response(utils::Socket& sock)
		{
			return recv_packet<PacketKind::Response, Ts...>(sock);
		}

		virtual void recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::SignupRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::SignupResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::LoginRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::LoginResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::LogoutRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::LogoutResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::MakeUserSetRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::MakeUserSetResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::GetUserSetsRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::GetUserSetsResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::GetMembersRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::GetMembersResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::DecryptRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::DecryptResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::UpdateRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::UpdateResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::DecryptParticipateRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::DecryptParticipateResponse& out) = 0;

		virtual void recv_request_data(utils::Socket& sock, pkt::SendDecryptionPartRequest& out) = 0;
		virtual void recv_response_data(utils::Socket& sock, pkt::SendDecryptionPartResponse& out) = 0;

	private:
		/**
		 * @enum senc::PacketReceiver::PacketKind
		 * @brief For internal use; Signified request or response.
		 */
		enum class PacketKind : std::uint8_t { Request, Response };

		/**
		 * @brief Receives data for specific packet.
		 * @tparam kind Whether should receive request or response.
		 * @tparam T Type of packet being received (packet struct).
		 * @return Received packet.
		 * @note Assumes valid input.
		 */
		template <PacketKind kind, typename T>
		inline T recv_packet_data(utils::Socket& sock)
		{
			T ret{};
			if constexpr (PacketKind::Request == kind)
				recv_request_data(sock, ret);
			else
				recv_response_data(sock, ret);
			return ret;
		}

		/**
		 * @brief Receives a packet of one of the given types.
		 * @tparam kind Whether should receive request or response.
		 * @tparam Ts Potential packet types (structs).
		 * @return Received packet, or `std::nullopt` if was of wrong type.
		 * @note Assuming any valid packet was sent, `sock` is guarenteed to be clear after call.
		 */
		template <PacketKind kind, typename... Ts>
		inline std::optional<utils::VariantOrSingular<Ts...>> recv_packet(utils::Socket& sock)
		{
			std::optional<utils::VariantOrSingular<Ts...>> ret;
			const auto code = sock.recv_connected_primitive<pkt::Code>();

			// for every type in Ts, check if its `CODE` is `code`, if so, set ret:
			([this, &sock, &ret, code]
			{
				if (!ret.has_value() && Ts::CODE == code) // only check code if didn't get packet already
				{
					ret.emplace(recv_packet_data<kind, Ts>(sock));
					return true;
				}
				return false;
			}() || ...);

			return ret;
		}
	};
}

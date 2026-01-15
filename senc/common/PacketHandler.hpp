/*********************************************************************
 * \file   PacketHandler.hpp
 * \brief  Contains abstract class PacketHandler.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/variants.hpp"
#include "../utils/Socket.hpp"
#include "packets.hpp"
#include <concepts>

namespace senc
{
	/**
	 * @class senc::PacketHandler
	 * @brief Abstraction of packet sending & receiving.
	 */
	class PacketHandler
	{
	public:
		using Self = PacketHandler;

		virtual ~PacketHandler() { }

		PacketHandler(Self&&) = default;

		/**
		 * @brief Sends given request with fitting code.
		 * @param packet Packet to send.
		 */
		template <typename T>
		inline void send_request(const T& packet)
		{
			_sock.send_connected_primitive(T::CODE);
			send_request_data(packet);
		}

		/**
		 * @brief Receives a request of one of the given types.
		 * @tparam Ts Potential packet types (structs).
		 * @return Received packet, or `std::nullopt` if was of wrong type.
		 * @note Assuming any valid packet was sent, `sock` is guarenteed to be clear after call.
		 */
		template <typename... Ts>
		inline std::optional<utils::VariantOrSingular<Ts...>> recv_request()
		{
			return recv_packet<PacketKind::Request, Ts...>();
		}

		/**
		 * @brief Sends given response with fitting code.
		 * @param packet Packet to send.
		 */
		template <typename T>
		inline void send_response(const T& packet)
		{
			_sock.send_connected_primitive(T::CODE);
			send_response_data(packet);
		}

		/**
		 * @brief Receives a response of one of the given types.
		 * @tparam Ts Potential packet types (structs).
		 * @return Received packet, or `std::nullopt` if was of wrong type.
		 * @note Assuming any valid packet was sent, `sock` is guarenteed to be clear after call.
		 */
		template <typename... Ts>
		inline std::optional<utils::VariantOrSingular<Ts...>> recv_response()
		{
			return recv_packet<PacketKind::Response, Ts...>();
		}

		virtual void send_response_data(const pkt::ErrorResponse& packet) = 0;
		virtual void recv_response_data(pkt::ErrorResponse& out) = 0;

		virtual void send_request_data(const pkt::SignupRequest& packet) = 0;
		virtual void recv_request_data(pkt::SignupRequest& out) = 0;

		virtual void send_response_data(const pkt::SignupResponse& packet) = 0;
		virtual void recv_response_data(pkt::SignupResponse& out) = 0;

		virtual void send_request_data(const pkt::LoginRequest& packet) = 0;
		virtual void recv_request_data(pkt::LoginRequest& out) = 0;

		virtual void send_response_data(const pkt::LoginResponse& packet) = 0;
		virtual void recv_response_data(pkt::LoginResponse& out) = 0;

		virtual void send_request_data(const pkt::LogoutRequest& packet) = 0;
		virtual void recv_request_data(pkt::LogoutRequest& out) = 0;

		virtual void send_response_data(const pkt::LogoutResponse& packet) = 0;
		virtual void recv_response_data(pkt::LogoutResponse& out) = 0;

		virtual void send_request_data(const pkt::MakeUserSetRequest& packet) = 0;
		virtual void recv_request_data(pkt::MakeUserSetRequest& out) = 0;

		virtual void send_response_data(const pkt::MakeUserSetResponse& packet) = 0;
		virtual void recv_response_data(pkt::MakeUserSetResponse& out) = 0;

		virtual void send_request_data(const pkt::GetUserSetsRequest& packet) = 0;
		virtual void recv_request_data(pkt::GetUserSetsRequest& out) = 0;

		virtual void send_response_data(const pkt::GetUserSetsResponse& packet) = 0;
		virtual void recv_response_data(pkt::GetUserSetsResponse& out) = 0;

		virtual void send_request_data(const pkt::GetMembersRequest& packet) = 0;
		virtual void recv_request_data(pkt::GetMembersRequest& out) = 0;

		virtual void send_response_data(const pkt::GetMembersResponse& packet) = 0;
		virtual void recv_response_data(pkt::GetMembersResponse& out) = 0;

		virtual void send_request_data(const pkt::DecryptRequest& packet) = 0;
		virtual void recv_request_data(pkt::DecryptRequest& out) = 0;

		virtual void send_response_data(const pkt::DecryptResponse& packet) = 0;
		virtual void recv_response_data(pkt::DecryptResponse& out) = 0;

		virtual void send_request_data(const pkt::UpdateRequest& packet) = 0;
		virtual void recv_request_data(pkt::UpdateRequest& out) = 0;

		virtual void send_response_data(const pkt::UpdateResponse& packet) = 0;
		virtual void recv_response_data(pkt::UpdateResponse& out) = 0;

		virtual void send_request_data(const pkt::DecryptParticipateRequest& packet) = 0;
		virtual void recv_request_data(pkt::DecryptParticipateRequest& out) = 0;

		virtual void send_response_data(const pkt::DecryptParticipateResponse& packet) = 0;
		virtual void recv_response_data(pkt::DecryptParticipateResponse& out) = 0;

		virtual void send_request_data(const pkt::SendDecryptionPartRequest& packet) = 0;
		virtual void recv_request_data(pkt::SendDecryptionPartRequest& out) = 0;

		virtual void send_response_data(const pkt::SendDecryptionPartResponse& packet) = 0;
		virtual void recv_response_data(pkt::SendDecryptionPartResponse& out) = 0;

	protected:
		utils::Socket& _sock;

		PacketHandler(utils::Socket& sock) : _sock(sock) { }

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
		inline T recv_packet_data()
		{
			T ret{};
			if constexpr (PacketKind::Request == kind)
				recv_request_data(ret);
			else
				recv_response_data(ret);
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
		inline std::optional<utils::VariantOrSingular<Ts...>> recv_packet()
		{
			std::optional<utils::VariantOrSingular<Ts...>> ret;
			const auto code = _sock.recv_connected_primitive<pkt::Code>();

			// for every type in Ts, check if its `CODE` is `code`, if so, set ret:
			([this, &ret, code]
			{
				if (!ret.has_value() && Ts::CODE == code) // only check code if didn't get packet already
				{
					ret.emplace(this->recv_packet_data<kind, Ts>());
					return true;
				}
				return false;
			}() || ...);

			return ret;
		}
	};

	/**
	 * @concept senc::PacketHandlerImpl
	 * @brief Looks for a typename which implements `PacketHandler` for both client and server side.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept PacketHandlerImpl = std::derived_from<Self, PacketHandler> &&
		std::move_constructible<Self> &&
		requires(utils::Socket& sock)
		{
			{ Self::server(sock) } -> std::same_as<Self>;
			{ Self::client(sock) } -> std::same_as<Self>;
		};
}

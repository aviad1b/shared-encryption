/*********************************************************************
 * \file   QueuedPacketHandler_impl.hpp
 * \brief  Implementation of `QueuedPacketHandler` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "QueuedPacketHandler.hpp"

namespace senc
{
	template <PacketHandlerImpl T>
	inline QueuedPacketHandler<T>::Self QueuedPacketHandler<T>::server(
		utils::Socket& sock,
		std::function<void(Underlying&)> onQueueEmpty,
		std::chrono::milliseconds delay)
	{
		return Underlying::server(sock);
	}

	template <PacketHandlerImpl T>
	inline QueuedPacketHandler<T>::Self QueuedPacketHandler<T>::client(
		utils::Socket& sock,
		std::function<void(Underlying&)> onQueueEmpty,
		std::chrono::milliseconds delay)
	{
		return Underlying::client(sock);
	}

	template <PacketHandlerImpl T>
	inline const IPacketHandlerSyncData& QueuedPacketHandler<T>::get_sync_data() const
	{
		return this->_underlying.get_sync_data();
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::ErrorResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::ErrorResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::SignupRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::SignupRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::SignupResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::SignupResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::LoginRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::LoginRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::LoginResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::LoginResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::LogoutRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::LogoutRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::LogoutResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::LogoutResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::MakeUserSetRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::MakeUserSetRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::MakeUserSetResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::MakeUserSetResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::GetUserSetsRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::GetUserSetsRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::GetUserSetsResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::GetUserSetsResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::GetMembersRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::GetMembersRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::GetMembersResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::GetMembersResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::DecryptRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::DecryptRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::DecryptResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::DecryptResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::UpdateRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::UpdateRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::UpdateResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::UpdateResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::DecryptParticipateRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::DecryptParticipateRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::DecryptParticipateResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::DecryptParticipateResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_request_data(const pkt::SendDecryptionPartRequest& packet)
	{
		queue_request(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_request_data(pkt::SendDecryptionPartRequest& out)
	{
		this->_underlying.recv_request_data(out);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::send_response_data(const pkt::SendDecryptionPartResponse& packet)
	{
		queue_response(packet);
	}

	template <PacketHandlerImpl T>
	inline void QueuedPacketHandler<T>::recv_response_data(pkt::SendDecryptionPartResponse& out)
	{
		this->_underlying.recv_response_data(out);
	}

	template <PacketHandlerImpl T>
	inline QueuedPacketHandler<T>::QueuedPacketHandler(Underlying&& underlying,
													   std::function<void(Underlying&)> onQueueEmpty,
													   std::chrono::milliseconds delay)
		: _underlying(std::move(underlying)),
		  _onQueueEmpty(onQueueEmpty),
		  _delay(delay) { }
}

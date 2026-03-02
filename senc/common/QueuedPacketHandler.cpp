/*********************************************************************
 * \file   QueuedPacketHandler.cpp
 * \brief  Implementation of `QueuedPacketHandler` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "QueuedPacketHandler.hpp"

namespace senc
{
	QueuedPacketHandler::QueuedPacketHandler(Self&& other) noexcept
		: Base(std::move(other)),
		  _underlying(std::move(other._underlying)),
		  _onQueueEmpty(std::move(other._onQueueEmpty)),
		  _delay(std::move(other._delay)),
		  _nextTicket(other._nextTicket),
		  _ticketBeingServed(other._ticketBeingServed),
		  _queueThread(&Self::queue_thread, this)
	{
		// signal other to stop (awaits operations finish)
		other._sync.stop = true;
	}

	QueuedPacketHandler::~QueuedPacketHandler()
	{
		_sync.stop = true;
	}

	QueuedPacketHandler::Self QueuedPacketHandler::server(
		utils::Socket& sock,
		std::function<void(PacketHandler&)> onQueueEmpty,
		std::chrono::milliseconds delay,
		ServerPacketHandlerFactory underlyingFactory)
	{
		return Self(sock, underlyingFactory(sock), onQueueEmpty, delay);
	}

	QueuedPacketHandler::Self QueuedPacketHandler::client(
		utils::Socket& sock,
		std::function<void(PacketHandler&)> onQueueEmpty,
		std::chrono::milliseconds delay,
		ClientPacketHandlerFactory underlyingFactory)
	{
		return Self(sock, underlyingFactory(sock), onQueueEmpty, delay);
	}

	const IPacketHandlerSyncData& QueuedPacketHandler::get_sync_data() const
	{
		return this->_underlying->get_sync_data();
	}

	void QueuedPacketHandler::send_response_data(const pkt::ErrorResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::ErrorResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::SignupRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::SignupRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::SignupResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::SignupResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::LoginRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::LoginRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::LoginResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::LoginResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::LogoutRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::LogoutRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::LogoutResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::LogoutResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::MakeUserSetRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::MakeUserSetRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::MakeUserSetResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::MakeUserSetResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::GetUserSetsRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::GetUserSetsRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::GetUserSetsResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::GetUserSetsResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::GetMembersRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::GetMembersRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::GetMembersResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::GetMembersResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::DecryptRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::DecryptRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::DecryptResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::DecryptResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::UpdateRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::UpdateRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::UpdateResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::UpdateResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::DecryptParticipateRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::DecryptParticipateRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::DecryptParticipateResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::DecryptParticipateResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	void QueuedPacketHandler::send_request_data(const pkt::SendDecryptionPartRequest& packet)
	{
		queue_request(packet);
	}

	void QueuedPacketHandler::recv_request_data(pkt::SendDecryptionPartRequest& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_request_data(out);
	}

	void QueuedPacketHandler::send_response_data(const pkt::SendDecryptionPartResponse& packet)
	{
		queue_response(packet);
	}

	void QueuedPacketHandler::recv_response_data(pkt::SendDecryptionPartResponse& out)
	{
		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		this->_underlying->recv_response_data(out);
	}

	QueuedPacketHandler::QueuedPacketHandler(
		utils::Socket& sock,
		std::unique_ptr<PacketHandler>&& underlying,
		std::function<void(PacketHandler&)> onQueueEmpty,
		std::chrono::milliseconds delay)
		: Base(sock),
		  _underlying(std::move(underlying)),
		  _onQueueEmpty(onQueueEmpty),
		  _delay(delay),
		  _nextTicket(0), _ticketBeingServed(0),
		  _queueThread(&Self::queue_thread, this) { }

	void QueuedPacketHandler::queue_thread()
	{
		while (!_sync.stop)
		{
			std::this_thread::sleep_for(_delay);

			std::unique_lock lock(_sync.mtxQueue);

			// if someone has aquired a ticket that we haven't served yet
			if (_ticketBeingServed < _nextTicket)
			{
				// mark ticket as being served and notify all waiters
				// (each will check for correct ticket)
				++_ticketBeingServed;
				_sync.cvQueue.notify_all();
			}
			else
			{
				const std::lock_guard l1(_sync.mtxOnQueueEmpty);
				const std::lock_guard l2(_sync.mtxUnderlying);
				_onQueueEmpty(*_underlying);
			}
		}
		_sync.cvQueue.notify_all(); // wake up all remaining threads
	}

	void QueuedPacketHandler::wait_queue()
	{
		std::unique_lock lock(_sync.mtxQueue);

		const std::size_t myTicket = _nextTicket++;

		_sync.cvQueue.wait(
			lock,
			[this, myTicket]() { return this->_sync.stop || myTicket == this->_ticketBeingServed; }
		);
	}

	template <typename R>
	void QueuedPacketHandler::queue_request(R&& request)
	{
		wait_queue();
		if (_sync.stop)
			return;

		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		_underlying->send_request_data(request);
	}

	template <typename R>
	void QueuedPacketHandler::queue_response(R&& response)
	{
		wait_queue();
		if (_sync.stop)
			return;

		const std::lock_guard<std::mutex> lock(_sync.mtxUnderlying);
		_underlying->send_response_data(response);
	}
}

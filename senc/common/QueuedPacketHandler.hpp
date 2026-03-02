/*********************************************************************
 * \file   QueuedPacketHandler.hpp
 * \brief  Header of `QueuedPacketHandler` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "ServerPacketHandlerFactory.hpp"
#include "ClientPacketHandlerFactory.hpp"
#include "PacketHandler.hpp"
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

namespace senc
{
	/**
	 * @class senc::QueuedPacketHandler
	 * @brief Implementation of `senc::PacketHandler` which wraps another implementation with a packets queue.
	 */
	class QueuedPacketHandler : public PacketHandler
	{
	public:
		using Self = QueuedPacketHandler;
		using Base = PacketHandler;

		/**
		 * @brief Move constructor of queued packet handler.
		 */
		QueuedPacketHandler(Self&& other) noexcept;

		/**
		 * @brief Destructor of queued packet handler, stops queue.
		 */
		~QueuedPacketHandler();

		/**
		 * @brief Gets handler instance for server side.
		 * @param sock Socket to send and receive packets through.
		 * @param onQueueEmpty Function to run on underlying handler when queue is empty.
		 * @param delay Delay to wait between queue invocations.
		 * @param underlyingFactory Packet handler factory used to construct underlying handler.
		 * @throw ConnEstablishException If failed to establish connection.
		 */
		static Self server(utils::Socket& sock,
						   std::function<void(PacketHandler&)> onQueueEmpty,
						   std::chrono::milliseconds delay,
						   ServerPacketHandlerFactory underlyingFactory);

		/**
		 * @brief Gets handler instance for client side.
		 * @param sock Socket to send and receive packets through.
		 * @param onQueueEmpty Function to run on underlying handler when queue is empty.
		 * @param delay Delay to wait between queue invocations.
		 * @param underlyingFactory Packet handler factory used to construct underlying handler.
		 * @throw ConnEstablishException If failed to establish connection.
		 */
		static Self client(utils::Socket& sock,
						   std::function<void(PacketHandler&)> onQueueEmpty,
						   std::chrono::milliseconds delay,
						   ClientPacketHandlerFactory underlyingFactory);

		const IPacketHandlerSyncData& get_sync_data() const override;

		void send_response_data(const pkt::ErrorResponse& packet) override;
		void recv_response_data(pkt::ErrorResponse& out) override;

		void send_request_data(const pkt::SignupRequest& packet) override;
		void recv_request_data(pkt::SignupRequest& out) override;

		void send_response_data(const pkt::SignupResponse& packet) override;
		void recv_response_data(pkt::SignupResponse& out) override;

		void send_request_data(const pkt::LoginRequest& packet) override;
		void recv_request_data(pkt::LoginRequest& out) override;

		void send_response_data(const pkt::LoginResponse& packet) override;
		void recv_response_data(pkt::LoginResponse& out) override;

		void send_request_data(const pkt::LogoutRequest& packet) override;
		void recv_request_data(pkt::LogoutRequest& out) override;

		void send_response_data(const pkt::LogoutResponse& packet) override;
		void recv_response_data(pkt::LogoutResponse& out) override;

		void send_request_data(const pkt::MakeUserSetRequest& packet) override;
		void recv_request_data(pkt::MakeUserSetRequest& out) override;

		void send_response_data(const pkt::MakeUserSetResponse& packet) override;
		void recv_response_data(pkt::MakeUserSetResponse& out) override;

		void send_request_data(const pkt::GetUserSetsRequest& packet) override;
		void recv_request_data(pkt::GetUserSetsRequest& out) override;

		void send_response_data(const pkt::GetUserSetsResponse& packet) override;
		void recv_response_data(pkt::GetUserSetsResponse& out) override;

		void send_request_data(const pkt::GetMembersRequest& packet) override;
		void recv_request_data(pkt::GetMembersRequest& out) override;

		void send_response_data(const pkt::GetMembersResponse& packet) override;
		void recv_response_data(pkt::GetMembersResponse& out) override;

		void send_request_data(const pkt::DecryptRequest& packet) override;
		void recv_request_data(pkt::DecryptRequest& out) override;

		void send_response_data(const pkt::DecryptResponse& packet) override;
		void recv_response_data(pkt::DecryptResponse& out) override;

		void send_request_data(const pkt::UpdateRequest& packet) override;
		void recv_request_data(pkt::UpdateRequest& out) override;

		void send_response_data(const pkt::UpdateResponse& packet) override;
		void recv_response_data(pkt::UpdateResponse& out) override;

		void send_request_data(const pkt::DecryptParticipateRequest& packet) override;
		void recv_request_data(pkt::DecryptParticipateRequest& out) override;

		void send_response_data(const pkt::DecryptParticipateResponse& packet) override;
		void recv_response_data(pkt::DecryptParticipateResponse& out) override;

		void send_request_data(const pkt::SendDecryptionPartRequest& packet) override;
		void recv_request_data(pkt::SendDecryptionPartRequest& out) override;

		void send_response_data(const pkt::SendDecryptionPartResponse& packet) override;
		void recv_response_data(pkt::SendDecryptionPartResponse& out) override;

	private:
		/**
		 * @struct senc::QueuedPacketHandler::Sync
		 * @brief Holds synchronization fields for QueuedPacketHandler.
		 */
		struct Sync
		{
			std::mutex mtxUnderlying;
			std::mutex mtxOnQueueEmpty;
			std::mutex mtxQueue;
			std::condition_variable cvQueue;
			std::atomic_bool stop;

			Sync() = default;

			/**
			 * @brief Dummy move constructor of Sync: Awaits all mutexes to release, and sets stop to true.
			 * @details This is used to make sure all operations inished before moving QueuedPacketHandler.
			 */
			Sync(Sync&& other) : Sync()
			{
				const std::lock_guard<std::mutex> a(other.mtxUnderlying);
				const std::lock_guard<std::mutex> b(other.mtxOnQueueEmpty);
				const std::lock_guard<std::mutex> c(other.mtxQueue);
				other.stop = true;
			}
		};

		Sync _sync;
		std::unique_ptr<PacketHandler> _underlying;
		std::function<void(PacketHandler&)> _onQueueEmpty;
		std::chrono::milliseconds _delay;
		std::size_t _nextTicket;
		std::size_t _ticketBeingServed;
		std::jthread _queueThread;

		/**
		 * @brief Constructs queued packet handler from underlying handler instance.
		 * @param sock Socket to send and receive packets through.
		 * @param underlying Underlying handler instance.
		 * @param onQueueEmpty Function to run on underlying handler when queue is empty.
		 * @param delay Delay to wait between queue invocations.
		 */
		QueuedPacketHandler(utils::Socket& sock,
							std::unique_ptr<PacketHandler>&& underlying,
							std::function<void(PacketHandler&)> onQueueEmpty,
							std::chrono::milliseconds delay);

		/**
		 * @brief Thread periodically invoking queue (or onQueueEmpty if empty).
		 */
		void queue_thread();

		/**
		 * @brief Joins queue and waits turn (or waits stop if that happens first).
		 */
		void wait_queue();

		/**
		 * @brief Queues request to be sent.
		 * @param request Request to queue.
		 */
		template <typename R>
		void queue_request(R&& request);

		/**
		 * @brief Queues response to be sent.
		 * @param response Response to queue.
		 */
		template <typename R>
		void queue_response(R&& response);
	};
}

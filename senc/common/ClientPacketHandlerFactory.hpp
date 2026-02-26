/*********************************************************************
 * \file   ClientPacketHandlerFactory.hpp
 * \brief  Header of `ClientPacketHandlerFactory` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "PacketHandler.hpp"
#include <functional>
#include <concepts>
#include <memory>

namespace senc
{
	/**
	 * @class senc::ClientPacketHandlerFactory
	 * @brief Used for creating instances of a `PacketHandler` implementation for client side.
	 */
	class ClientPacketHandlerFactory
	{
	public:
		using Self = ClientPacketHandlerFactory;

		virtual ~ClientPacketHandlerFactory() { }

		/**
		 * @brief Constructs a new client-side packet handler instance from a socket reference.
		 * @param sock Reference to socket to send and receive packets through.
		 */
		std::unique_ptr<PacketHandler> operator()(utils::Socket& sock) const
		{
			return _make(sock);
		}

	protected:
		/**
		 * @brief Constructor of client packet handler factory.
		 * @param make A function which constructs a client packet handler instance from a socket reference.
		 */
		ClientPacketHandlerFactory(std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> make)
			: _make(make) { }

	private:
		std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> _make;
	};

	/**
	 * @class senc::ClientPacketHandlerImplFactory
	 * @brief Type erasure for `ClientPacketHandlerFactory`.
	 * @tparam T `PacketHandler` implementing type.
	 * @tparam Args Argument types used for construction for client packet handler in addition to socket.
	 * @note Args are *copied* into factory, not moved.
	 */
	template <typename T, typename... Args>
	requires PacketHandlerImpl<T, Args...>
	class ClientPacketHandlerImplFactory : public ClientPacketHandlerFactory
	{
	public:
		using Self = ClientPacketHandlerImplFactory<T, Args...>;
		using Base = ClientPacketHandlerFactory;

		/**
		 * @brief Default constructor of client packet handler factory.
		 */
		ClientPacketHandlerImplFactory(const Args&... args) : Base(
			[args...](utils::Socket& sock) { return std::make_unique<T>(T::client(sock, args...)); }
		) { }
	};
}

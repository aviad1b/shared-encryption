/*********************************************************************
 * \file   ServerPacketHandlerFactory.hpp
 * \brief  Header of `ServerPacketHandlerFactory` class.
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
	 * @class senc::ServerPacketHandlerFactory
	 * @brief Used for creating instances of a `PacketHandler` implementation for server side.
	 */
	class ServerPacketHandlerFactory
	{
	public:
		using Self = ServerPacketHandlerFactory;

		virtual ~ServerPacketHandlerFactory() { }

		/**
		 * @brief Constructs a new server-side packet handler instance from a socket reference.
		 * @param sock Reference to socket to send and receive packets through.
		 */
		std::unique_ptr<PacketHandler> operator()(utils::Socket& sock) const
		{
			return _make(sock);
		}

	protected:
		/**
		 * @brief Constructor of server packet handler factory.
		 * @param make A function which constructs a server packet handler instance from a socket reference.
		 */
		ServerPacketHandlerFactory(std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> make)
			: _make(make) { }

	private:
		std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> _make;
	};

	/**
	 * @class senc::ServerPacketHandlerImplFactory
	 * @brief Type erasure for `ServerPacketHandlerFactory`.
	 * @tparam T `PacketHandler` implementing type.
	 * @tparam Args Argument types used for construction for server packet handler in addition to socket.
	 * @note Args are *copied* into factory, not moved.
	 */
	template <typename T, typename... Args>
	requires PacketHandlerImpl<T, Args...>
	class ServerPacketHandlerImplFactory : public ServerPacketHandlerFactory
	{
	public:
		using Self = ServerPacketHandlerImplFactory<T, Args...>;
		using Base = ServerPacketHandlerFactory;

		/**
		 * @brief Default constructor of server packet handler factory.
		 */
		ServerPacketHandlerImplFactory(const Args&... args) : Base(
			[args...](utils::Socket& sock) { return std::make_unique<T>(T::server(sock, args...)); }
		) { }
	};
}

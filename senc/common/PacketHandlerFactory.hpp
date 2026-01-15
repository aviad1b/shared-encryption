/*********************************************************************
 * \file   PacketHandlerFactory.hpp
 * \brief  Contains PacketHandlerFactory class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "PacketHandler.hpp"
#include <functional>
#include <concepts>
#include <memory>

namespace senc
{
	/**
	 * @class senc::PacketHandlerFactory
	 * @brief Used for creating instances of a `PacketHandler` implementation.
	 * @tparam T `PacketHandler` implementing type, of `PacketHandler` itself for base.
	 */
	class PacketHandlerFactory
	{
	public:
		using Self = PacketHandlerFactory;

		virtual ~PacketHandlerFactory() { }

		/**
		 * @brief Constructs a new server-side packet handler instance from a socket reference.
		 * @param sock Reference to socket to send and receive packets through.
		 */
		std::unique_ptr<PacketHandler> new_server_packet_handler(utils::Socket& sock) const
		{
			return _server(sock);
		}

		/**
		 * @brief Constructs a new client-side packet handler instance from a socket reference.
		 * @param sock Reference to socket to send and receive packets through.
		 */
		std::unique_ptr<PacketHandler> new_client_packet_handler(utils::Socket& sock) const
		{
			return _client(sock);
		}

	protected:
		/**
		 * @brief Constructor of packet handler factory.
		 * @param server A function which constructs a server packet handler instance from a socket reference.
		 * @param client A function which constructs a client packet handler instance from a socket reference.
		 */
		PacketHandlerFactory(
			std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> server,
			std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> client
		) : _server(server), _client(client) { }

	private:
		std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> _server, _client;
	};

	/**
	 * @class senc::PacketHandlerImplFactory
	 * @brief Used for creating instances of a `PacketHandler` implementation.
	 * @tparam T `PacketHandler` implementing type, of `PacketHandler` itself for base.
	 */
	template <PacketHandlerImpl T>
	class PacketHandlerImplFactory : public PacketHandlerFactory
	{
	public:
		using Self = PacketHandlerImplFactory<T>;
		using Base = PacketHandlerFactory;

		/**
		 * @brief Default constructor of packet handler factory.
		 */
		PacketHandlerImplFactory() : Base(
			[](utils::Socket& sock) { return std::make_unique<T>(T::server(sock)); },
			[](utils::Socket& sock) { return std::make_unique<T>(T::client(sock)); }
		) { }
	};
}

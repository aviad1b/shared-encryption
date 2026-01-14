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
		 * @brief Constructs a new packet handler instance from a socket reference.
		 * @param sock Reference to socket to send and receive packets through.
		 */
		std::unique_ptr<PacketHandler> new_packet_handler(utils::Socket& sock)
		{
			return _f(sock);
		}

	protected:
		/**
		 * @brief Constructor of packet handler factory.
		 * @param f A function which constructs a packet handler instance from a socket reference.
		 */
		PacketHandlerFactory(std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> f)
			: _f(f) { }

	private:
		std::function<std::unique_ptr<PacketHandler>(utils::Socket&)> _f;
	};

	/**
	 * @class senc::PacketHandlerImplFactory
	 * @brief Used for creating instances of a `PacketHandler` implementation.
	 * @tparam T `PacketHandler` implementing type, of `PacketHandler` itself for base.
	 */
	template <std::derived_from<PacketHandler> T>
	requires std::constructible_from<T, utils::Socket&>
	class PacketHandlerImplFactory : public PacketHandlerFactory
	{
	public:
		using Self = PacketHandlerImplFactory<T>;
		using Base = PacketHandlerFactory;

		/**
		 * @brief Default constructor of packet handler factory.
		 */
		PacketHandlerFactory() : Base(std::make_unique<T>) { }
	};
}

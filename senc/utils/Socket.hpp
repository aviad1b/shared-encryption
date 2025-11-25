/*********************************************************************
 * \file   Socket.hpp
 * \brief  Contains socket-related declarations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <WinSock2.h> // has to be before <Windows.h>
#include <Windows.h>
#include <ws2ipdef.h>
#include <concepts>
#include <cstddef>
#include <string>
#include <vector>
#include <tuple>

#include "Exception.hpp"
#include "bytes.hpp"

namespace senc::utils
{
	/**
	 * @typedef senc::utils::port
	 * @brief Represents a transport port number.
	 */
	using Port = int;

	template <typename Self>
	concept IPType = std::copyable<Self> &&
		std::equality_comparable<Self> &&
		std::constructible_from<Self, typename Self::Underlying> &&
		requires(
			const Self self, typename Self::UnderlyingSockAddr* out,
			const typename Self::UnderlyingSockAddr underlyingSockAddr
		)
		{
			{ Self::UNDERLYING_ADDRESS_FAMILY } -> std::convertible_to<int>;
			{ Self::ANY } -> std::convertible_to<const Self&>;
			{ self.as_str() } noexcept -> std::convertible_to<const std::string&>;
			{ self.init_underlying(out, std::declval<Port>()) } noexcept;
			{ Self::from_underlying_sock_addr(underlyingSockAddr) } -> std::same_as<std::tuple<Self, Port>>;
		};

	/**
	 * @class senc::utils::IPv4
	 * @brief Represents an IPv4 address.
	 */
	class IPv4
	{
	public:
		using Self = IPv4;
		using Underlying = struct in_addr;
		using UnderlyingSockAddr = struct sockaddr_in;
		static constexpr int UNDERLYING_ADDRESS_FAMILY = AF_INET;

		/**
		 * @brief Used for binding socket to any address.
		 */
		static const Self ANY;

		/**
		 * @brief Constructs an IPv4 address from underlying struct.
		 * @param underlying Underlying struct instance.
		 */
		explicit IPv4(const Underlying& underlying);

		/**
		 * @brief Gets IPv4 address and port from underlying sockaddr struct.
		 */
		static std::tuple<Self, Port> from_underlying_sock_addr(
			const UnderlyingSockAddr& underlyingSockAddr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address.
		 * @throw senc::utils::SocketException If `addr` is not a valid IPv4 address.
		 */
		IPv4(const char* addr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address.
		 * @throw senc::utils::SocketException If `addr` is not a valid IPv4 address.
		 */
		IPv4(const std::string& addr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address (moved).
		 * @throw senc::utils::SocketException If `addr` is not a valid IPv4 address.
		 */
		IPv4(std::string&& addr);

		/**
		 * @brief Copy constructor of IPv4 address.
		 */
		IPv4(const Self&) = default;

		/**
		 * @brief Copy assignment operator of IPv4 address.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Move constructor of IPv4 address.
		 */
		IPv4(Self&&) = default;

		/**
		 * @brief Move assignment operator of IPv4 Address.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Compares the IPv4 address to another.
		 * @param other Other IPv4 address to compare to.
		 * @param `true` if `*this` is the same address as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const noexcept;

		/**
		 * @brief Gets string representation of IPv4 address.
		 * @return String representation of IPv4 address.
		 */
		const std::string& as_str() const noexcept;

		/**
		 * @brief Initializes underlying library's struct based on IPv4 address and port.
		 * @param out Address of underlying library's struct to initialize.
		 * @param port Port to initialize based on.
		 */
		void init_underlying(UnderlyingSockAddr* out, Port port) const noexcept;

	private:
		Underlying _addr{};
		std::string _addrStr;
	};
	static_assert(IPType<IPv4>);

	/**
	 * @class senc::utils::IPv6
	 * @brief Represents an IPv6 address.
	 */
	class IPv6
	{
	public:
		using Self = IPv6;
		using Underlying = struct in6_addr;
		using UnderlyingSockAddr = struct sockaddr_in6;
		static constexpr int UNDERLYING_ADDRESS_FAMILY = AF_INET6;

		/**
		 * @brief Used for binding socket to any address.
		 */
		static const Self ANY;

		/**
		 * @brief Constructs an IPv6 address from underlying struct.
		 * @param underlying Underlying struct instance.
		 */
		explicit IPv6(const Underlying& underlying);

		/**
		 * @brief Gets IPv6 address and port from underlying sockaddr struct.
		 */
		static std::tuple<Self, Port> from_underlying_sock_addr(
			const UnderlyingSockAddr& underlyingSockAddr);

		/**
		 * @brief Constructs an IPv6 address from string representation.
		 * @param addr String representation of IPv6 address.
		 * @throw senc::utils::SocketException If `addr` is not a valid IPv6 address.
		 */
		IPv6(const char* addr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address.
		 * @throw senc::utils::SocketException If `addr` is not a valid IPv6 address.
		 */
		IPv6(const std::string& addr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address (moved).
		 * @throw senc::utils::SocketException If `addr` is not a valid IPv6 address.
		 */
		IPv6(std::string&& addr);

		/**
		 * @brief Copy constructor of IPv4 address.
		 */
		IPv6(const Self&) = default;

		/**
		 * @brief Copy assignment operator of IPv4 address.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Move constructor of IPv4 address.
		 */
		IPv6(Self&&) = default;

		/**
		 * @brief Move assignment operator of IPv4 Address.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Compares the IPv4 address to another.
		 * @param other Other IPv4 address to compare to.
		 * @param `true` if `*this` is the same address as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const noexcept;

		/**
		 * @brief Gets string representation of IPv4 address.
		 * @return String representation of IPv4 address.
		 */
		const std::string& as_str() const noexcept;

		/**
		 * @brief Initializes underlying library's struct based on IPv4 address and port.
		 * @param out Address of underlying library's struct to initialize.
		 * @param port Port to initialize based on.
		 * @throw senc::utils::SocketException On failure.
		 */
		void init_underlying(UnderlyingSockAddr* out, Port port) const noexcept;

	private:
		Underlying _addr{};
		std::string _addrStr;
	};
	static_assert(IPType<IPv6>);

	/**
	 * @class senc::utils::SocketException
	 * @brief Type of exceptions thrown on socket errors.
	 */
	class SocketException : public Exception
	{
	public:
		using Self = SocketException;
		using Base = Exception;

		SocketException(const std::string& msg);

		SocketException(std::string&& msg);

		SocketException(const std::string& msg, const std::string& info);
		
		SocketException(std::string&& msg, const std::string& info);

		SocketException(const Self&) = default;

		Self& operator=(const Self&) = default;

		SocketException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};

	class SocketInitializer
	{
		friend class Socket;

	public:
		~SocketInitializer();

	private:
		SocketInitializer();
	};

	/**
	 * @class senc::utils::Socket
	 * @brief Base class of all socket classes.
	 */
	class Socket
	{
		friend class SocketInitializer;

	public:
		using Self = Socket;

		Socket(const Self&) = delete;

		Self& operator=(const Self&) = delete;

		/**
		 * @brief Base destructor of sockets, closes socket.
		 */
		virtual ~Socket();

		/**
		 * @return `true` if socket closed, otherwise `false`.
		 */
		bool is_closed() const;

		/**
		 * @brief Determines if socket is already connected to an address.
		 * @return `true` if socket is connected, otherwise `false`.
		 */
		bool is_connected() const;

		/**
		 * @brief Sends binary data through (a connected) socket.
		 * @param data Pointer to sequential binary data.
		 * @param size Size of data (byte count).
		 * @throw senc::utils::SocketException On failure.
		 */
		void send_connected(const void* data, std::size_t size);

		/**
		 * @brief Sends binary data through (a connected) socket.
		 * @param data Binary data to send.
		 * @throw senc::utils::SocketException On failure.
		 */
		void send_connected(const HasByteData auto& data);

		/**
		 * @brief Sends string data through (a connected) socket.
		 * @param data String data to send.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <StringType Str>
		void send_connected_str(const Str& data);

		/**
		 * @brief Recieves binary data through (a connected) socket.
		 * @param maxsize Maximum amount of bytes to recieve.
		 * @return Recieved data.
		 * @throw senc::utils::SocketException On failure.
		 */
		Buffer recv_connected(std::size_t maxsize);

		/**
		 * @brief Recieves binary data through (a connected) socket.
		 * @param out Address to read received data into.
		 * @param maxsize Maximum amount of bytes to recieve.
		 * @return Amount of bytes read.
		 * @throw senc::utils::SocketException On failure.
		 */
		std::size_t recv_connected_into(void* out, std::size_t maxsize);

		/**
		 * @brief Recieves binary data through (a connected) socket.
		 * @param out An object holding mutable byte data to read received data into.
		 * @return Amount of bytes read.
		 * @throw senc::utils::SocketException On failure.
		 * @note Reads `out.size()` bytes at max.
		 */
		std::size_t recv_connected_into(HasMutableByteData auto& out);

	protected:
		using Underlying = SOCKET;
		static constexpr Underlying UNDERLYING_NO_SOCK = INVALID_SOCKET;

		Underlying _sock;
		bool _isConnected;

		/**
		 * @brief Constructor of base socket from underlying library's socket.
		 * @throw senc::utils::SocketException On failure.
		 */
		Socket(Underlying sock, bool isConnected = false);

		/**
		 * @brief Socket move constructor.
		 */
		Socket(Self&& other);

		/**
		 * @brief Socket move assignment operator.
		 */
		Self& operator=(Self&& other);

		/**
		 * @brief Closes socket connection.
		 */
		void close();

		/**
		 * @brief Outputs leftover data from previous recvs.
		 * @param out Address to output to.
		 * @param maxsize Maximum byte count to output from leftover data.
		 * @return Amount of bytes outputted.
		 */
		std::size_t out_leftover_data(void* out, std::size_t maxsize);

		static std::string get_last_sock_err();

	private:
		static const SocketInitializer SOCKET_INITIALIZER;
		Buffer _buffer; // for leftover data
	};

	/**
	 * @class senc::utils::ConnectableSocket
	 * @brief Base class of all sockets that can be connected to an address.
	 * @tparam IP Type of IP addresses (e.g. `senc::utils::IPv4`).
	 */
	template <IPType IP>
	class ConnectableSocket : public Socket
	{
	public:
		using Self = ConnectableSocket<IP>;
		using Base = Socket;

		virtual ~ConnectableSocket() = default;

		/**
		 * @brief Closes socket connection.
		 */
		void close();

		/**
		 * @brief Connects socket to a given IP address and port.
		 * @param addr IP address to connect to.
		 * @param port Transport-level port to connect to.
		 * @throw senc::utils::SocketException On failure.
		 */
		void connect(const IP& addr, Port port);

		/**
		 * @brief Binds socket to a given port.
		 * @param port Port to bind socket to.
		 * @throw senc::utils::SocketException On failure.
		 */
		void bind(Port port);

		/**
		 * @brief Binds socket to a given IP address and port.
		 * @param addr IP address to bind socket to.
		 * @param port Port to bind socket to.
		 * @throw senc::utils::SocketException On failure.
		 */
		void bind(const IP& addr, Port port);

	protected:
		using Underlying = Base::Underlying;

		/**
		 * @brief Constructor of connectable socket from underlying library's parameters.
		 * @throw senc::utils::SocketException On failure.
		 */
		ConnectableSocket(Underlying sock, bool isConnected = false);

		/**
		 * @brief Connectable socket move constructor.
		 */
		ConnectableSocket(Self&&) = default;

		/**
		 * @brief Connectable socket move assignment operator.
		 */
		Self& operator=(Self&&) = default;
	};

	/**
	 * @class senc::utils::TcpSocket
	 * @brief Object used for managing a single TCP socket connection.
	 * @tparam IP Type of IP addresses (e.g. `senc::utils::IPv4`).
	 */
	template <IPType IP>
	class TcpSocket : public ConnectableSocket<IP>
	{
	public:
		using Self = TcpSocket<IP>;
		using Base = ConnectableSocket<IP>;

		virtual ~TcpSocket() = default;

		/**
		 * @brief Constructs a TCP socket.
		 * @throw senc::utils::SocketException On failure.
		 */
		TcpSocket();

		/**
		 * @brief Constructs a TCP socket, and connects it to given IP address and port.
		 * @param addr IP address to connect socket to.
		 * @param port TCP port to connect socket to.
		 * @throw senc::utils::SocketException On failure.
		 */
		TcpSocket(const IP& addr, Port port);

		/**
		 * @brief TCP socket move constructor.
		 */
		TcpSocket(Self&&) = default;

		/**
		 * @brief TCP socket move assignment operator.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Begins listening for clients.
		 * @throw senc::utils::SocketException On failure.
		 */
		void listen();

		/**
		 * @brief Accepts new client (after listening) - blocking operation.
		 * @return New TCP socket connected to accepted client.
		 * @throw senc::utils::SocketException On failure.
		 */
		Self accept();

	protected:
		using Underlying = Base::Underlying;

		/**
		 * @brief Constructor of TCP socket from underlying library's parameters.
		 * @throw senc::utils::SocketException On failure.
		 */
		TcpSocket(Underlying sock, bool isConnected = false);
	};

	/**
	 * @class senc::utils::UdpSocket
	 * @brief Object used for managing a single UDP socket connection.
	 * @tparam IP Type of IP addresses (e.g. `senc::utils::IPv4`).
	 */
	template <IPType IP>
	class UdpSocket : public ConnectableSocket<IP>
	{
	public:
		using Self = UdpSocket<IP>;
		using Base = ConnectableSocket<IP>;

		virtual ~UdpSocket() = default;

		/**
		 * @brief Constructs a UDP socket.
		 * @throw senc::utils::SocketException On failure.
		 */
		UdpSocket();

		/**
		 * @brief UDP socket move constructor.
		 */
		UdpSocket(Self&&) = default;

		/**
		 * @brief UDP socket move assignment operator.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Disconnects socket from (previously-connected-to) address and port.
		 * @throw senc::utils::SocketException On failure.
		 */
		void disconnect();

		/**
		 * @brief Sends data to given IP address and port.
		 * @note Requires socket to be disconnected.
		 * @param data Pointer to sequential binary data.
		 * @param size Size of data (byte count).
		 * @param addr IP address to send data to.
		 * @param port UDP port to send data to.
		 * @throw senc::utils::SocketException On failure.
		 */
		void send_to(const void* data, std::size_t size, const IP& addr, Port port);

		/**
		 * @brief Sends data to given IP address and port.
		 * @note Requires socket to be disconnected.
		 * @param data Binary data to send.
		 * @param addr IP address to send data to.
		 * @param port UDP port to send data to.
		 * @throw senc::utils::SocketException On failure.
		 */
		void send_to(const HasByteData auto& data, const IP& addr, Port port);

		struct recv_from_ret_t
		{
			Buffer data;
			IP addr;
			Port port;
		};

		/**
		 * @brief Recieves data through socket.
		 * @note Requires socket to be disconnected.
		 * @param maxsize Maximum amount of bytes to recieve.
		 * @return Recieved data.
		 * @throw senc::utils::SocketException On failure.
		 */
		recv_from_ret_t recv_from(std::size_t maxsize);

		struct recv_from_into_ret_t
		{
			std::size_t count;
			IP addr;
			Port port;
		};

		/**
		 * @brief Recieves data through socket.
		 * @param out Address to read received data into.
		 * @param maxsize Maximum amount of bytes to recieve.
		 * @return Amount of bytes read.
		 * @throw senc::utils::SocketException On failure.d
		 */
		recv_from_into_ret_t recv_from_into(void* out, std::size_t maxsize);

		/**
		 * @brief Recieves data through socket.
		 * @param out An object holding mutable byte data to read received data into.
		 * @return Amount of bytes read.
		 * @throw senc::utils::SocketException On failure.
		 * @note Reads `out.size()` bytes at max.
		 */
		recv_from_into_ret_t recv_from_into(HasMutableByteData auto& out);

	protected:
		using Underlying = Base::Underlying;

		/**
		 * @brief Constructor of UDP socket from underlying library's parameters.
		 * @throw senc::utils::SocketException On failure.
		 */
		UdpSocket(Underlying sock, bool isConnected = false);
	};
}

#include "Socket_impl.hpp"

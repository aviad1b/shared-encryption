/*********************************************************************
 * \file   Socket.hpp
 * \brief  Contains socket-related declarations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include "env.hpp"

#ifdef SENC_WINDOWS
#include "../utils/winapi_patch.hpp"
#include <ws2ipdef.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#endif

#include <concepts>
#include <cstddef>
#include <string>
#include <vector>
#include <tuple>
#include <bit>

#include "Exception.hpp"
#include "ModInt.hpp"
#include "bytes.hpp"

namespace senc::utils
{
	/**
	 * @typedef senc::utils::port
	 * @brief Represents a transport port number.
	 */
	using Port = unsigned short;

	/**
	 * @concept senc::utils::IPType
	 * @brief Looks for a typename that can be used as an IP address (e.g. IPv4, IPv6).
	 * @tparam Self Examined typename.
	 */
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
			{ Self::any() } -> std::convertible_to<const Self&>;
			{ Self::loopback() } -> std::convertible_to<const Self&>;
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
		 * @brief Gets IP used for binding socket to any address.
		 */
		static const Self& any();

		/**
		 * @brief Gets IP used for loopback.
		 */
		static const Self& loopback();

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
		 * @brief Gets IP used for binding socket to any address.
		 */
		static const Self& any();

		/**
		 * @brief Gets IP used for loopback.
		 */
		static const Self& loopback();

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

		SocketException(const std::string& msg) : Base(msg) { }

		SocketException(std::string&& msg) : Base(std::move(msg)) { }

		SocketException(const std::string& msg, const std::string& info) : Base(msg, info) { }
		
		SocketException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) { }

		SocketException(const Self&) = default;

		Self& operator=(const Self&) = default;

		SocketException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};

	/**
	 * @class senc::utils::SocketUtils
	 * @brief Contains utility functions for sockets.
	 */
	class SocketUtils
	{
	public:
		/**
		 * @brief Gets last socket error from system.
		 * @return Last socket error message.
		 */
		static std::string get_last_sock_err();
	};

	/**
	 * @class senc::utils::SocketInitializer
	 * @brief Utility class used for socket environment initialization.
	 *		  Not to be used externally.
	 */
	class SocketInitializer
	{
		friend class Socket;

	public:
		~SocketInitializer();

	private:
		SocketInitializer();

		static const SocketInitializer SOCKET_INITIALIZER;
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

		static constexpr std::endian DEFAULT_ENDIANESS = std::endian::big;

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
		 * @tparam endianess Endianess to use while sending (`big` to keep as-is, `little` to reverse).
		 * @param data String data to send.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <std::endian endianess = DEFAULT_ENDIANESS>
		void send_connected_str(const StringType auto& data);

		/**
		 * @brief Sends a simple value through (a connected) socket.
		 * @tparam endianess Endianess to use while sending (`big` to keep as-is, `little` to reverse).
		 * @param value Value to send.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <std::endian endianess = DEFAULT_ENDIANESS>
		void send_connected_primitive(auto value)
		requires (std::is_fundamental_v<std::remove_cvref_t<decltype(value)>> || 
			std::is_enum_v<std::remove_cvref_t<decltype(value)>>);

		/**
		 * @brief Sends a ModInt value through (a connected) socket.
		 * @tparam endianess Endianess to use while sending (`big` to keep as-is, `little` to reverse).
		 * @param value Value to send.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <std::endian endianess = DEFAULT_ENDIANESS>
		void send_connected_modint(const ModIntType auto& value);

		/**
		 * @brief Sends an object instance throught (a connected) socket.
		 * @tparam Obj Object type, must have a `to_bytes` method.
		 * @param obj Object instance to send.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <HasToBytes Obj>
		void send_connected_object(const Obj& obj);

		/**
		 * @brief Sends a value through (a connected) socket, using the fitting method.
		 * @tparam endianess Endianess to use while sending (`big` to keep as-is, `little` to reverse).
		 * @param value Value to send.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <std::endian endianess = DEFAULT_ENDIANESS>
		void send_connected_value(const auto& value)
		requires (HasByteData<std::remove_cvref_t<decltype(value)>> ||
			StringType<std::remove_cvref_t<decltype(value)>> ||
			std::is_fundamental_v<std::remove_cvref_t<decltype(value)>> ||
			std::is_enum_v<std::remove_cvref_t<decltype(value)>> ||
			ModIntType<std::remove_cvref_t<decltype(value)>> ||
			HasToBytes<std::remove_cvref_t<decltype(value)>> ||
			TupleLike<std::remove_cvref_t<decltype(value)>>);

		/**
		 * @brief Sends values through (a connected) socket, using the fitting method for each.
		 * @tparam endianess Endianess to use while sending (`big` to keep as-is, `little` to reverse).
		 * @param values Values to send.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <std::endian endianess = DEFAULT_ENDIANESS>
		void send_connected_values(const TupleLike auto& values);

		/**
		 * @brief Recieves binary data through (a connected) socket.
		 * @param maxsize Maximum amount of bytes to recieve.
		 * @return Recieved data.
		 * @throw senc::utils::SocketException On failure.
		 */
		Buffer recv_connected(std::size_t maxsize);

		/**
		 * @brief Recieves binary data through (a connected) socket.
		 * @param size Exact amount of bytes to recieve.
		 * @return Recieved data.
		 * @throw senc::utils::SocketException On failure.
		 */
		Buffer recv_connected_exact(std::size_t size);

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
		 * @param out Address to read received data into.
		 * @param size Exact amount of bytes to recieve.
		 * @throw senc::utils::SocketException On failure.
		 */
		void recv_connected_exact_into(void* out, std::size_t size);

		/**
		 * @brief Recieves binary data through (a connected) socket.
		 * @param out An object holding mutable byte data to read received data into.
		 * @return Amount of bytes read.
		 * @throw senc::utils::SocketException On failure.
		 * @note Reads `out.size()` bytes at max.
		 */
		std::size_t recv_connected_into(HasMutableByteData auto& out);

		/**
		 * @brief Recieves binary data through (a connected) socket.
		 * @param out An object holding mutable byte data to read received data into.
		 * @throw senc::utils::SocketException On failure.
		 * @note Reads *exactly* `out.size()` bytes.
		 */
		void recv_connected_exact_into(HasMutableByteData auto& out);

		/**
		 * @brief Recieves string data through (a connected) socket.
		 * @tparam Str String data type to recieve (same as one sent on other end).
		 * @tparam endianess Endianess to use while sending (`big` to keep as-is, `little` to reverse).
		 * @return Read string data.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <StringType Str = std::string,
				  std::endian endianess = std::endian::native,
				  std::size_t chunkSize = 32>
		Str recv_connected_str();

		/**
		 * @brief Recieves simple value through (a connected) socket.
		 * @tparam endianess Endianess to use while receiving (`big` to keep as-is, `little` to reverse).
		 * @tparam T Value type (fundamental or enum).
		 * @return Read value.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <typename T, std::endian endianess = DEFAULT_ENDIANESS>
		requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
		T recv_connected_primitive();

		/**
		 * @brief Receives a ModInt instance.
		 * @tparam endianess Endianess to use while receiving (`big` to keep as-is, `little` to reverse).
		 * @tparam T ModInt type.
		 * @return Read value.
		 * @throw senc::utils::ModException if valie is invalid.
		 * @throw senc::utils::SocketException On other failure.
		 */
		template <ModIntType T, std::endian endianess = DEFAULT_ENDIANESS>
		T recv_connected_modint();

		/**
		 * @brief Receives an object instance through (a connected) socket.
		 * @tparam Obj Object type, must have a `from_bytes` and a `bytes_size` method.
		 * @return Read object instance.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <HasFromBytes T>
		requires HasFixedBytesSize<T>
		T recv_connected_obj();

		/**
		 * @brief Recieves value through (a connected) socket, using the fitting method.
		 * @tparam T Value type.
		 * @tparam endianess Endianess to use while sending (`big` to keep as-is, `little` to reverse).
		 * @param out Reference to store read value to.
		 */
		template <typename T,
				  std::endian endianess = DEFAULT_ENDIANESS,
				  std::size_t chunkSize = 32>
		requires (HasMutableByteData<T> || StringType<T> || 
			std::is_fundamental_v<T> || std::is_enum_v<T> ||
			ModIntType<T> ||
			(HasFromBytes<T> && HasFixedBytesSize<T>) ||
			TupleLike<T>)
		void recv_connected_value(T& out);

		/**
		 * @brief Recieves values through (a connected) socket, using the fitting method for each.
		 * @param out Reference to store read values to.
		 * @throw senc::utils::SocketException On failure.
		 */
		template <TupleLike Tpl,
				  std::endian endianess = DEFAULT_ENDIANESS,
				  std::size_t chunkSize = 32>
		void recv_connected_values(Tpl& values);

	protected:
#ifdef SENC_WINDOWS
		using Underlying = SOCKET;
		static constexpr Underlying UNDERLYING_NO_SOCK = INVALID_SOCKET;
#else
		using Underlying = int;
		static constexpr Underlying UNDERLYING_NO_SOCK = -1;
#endif

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

		static bool underlying_has_data(Underlying sock);

	private:
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
		std::pair<Self, std::tuple<IP, Port>> accept();

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
		 * @brief Receives data through socket.
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
		 * @brief Receives data through socket.
		 * @param out Address to read received data into.
		 * @param maxsize Maximum amount of bytes to recieve.
		 * @return Amount of bytes read as well as IP address and port from which received.
		 * @throw senc::utils::SocketException On failure.d
		 */
		recv_from_into_ret_t recv_from_into(void* out, std::size_t maxsize);

		/**
		 * @brief Recieves data through socket.
		 * @param out An object holding mutable byte data to read received data into.
		 * @return Amount of bytes read as well as IP address and port from which received.
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

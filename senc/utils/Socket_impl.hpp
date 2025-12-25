/*********************************************************************
 * \file   Socket_impl.hpp
 * \brief  Template implementation of socket-related classes.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Socket.hpp"

#include <algorithm>

namespace senc::utils
{
	inline void Socket::send_connected(const HasByteData auto& data)
	{
		// Note: We assume here that data.size() does not surpass int limit.
		return send_connected(data.data(), data.size());
	}

	template <StringType Str>
	inline void Socket::send_connected_str(const Str& data)
	{
		using C = typename Str::value_type;
		return send_connected(data.c_str(), (data.size() + 1) * sizeof(C));
	}

	template <typename T>
	requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
	inline void Socket::send_connected_primitive(T value)
	{
		send_connected(&value, sizeof(value));
	}

	template <ModIntType T>
	inline void Socket::send_connected_modint(const T& value)
	{
		using Int = typename T::Int;
		send_connected_primitive(static_cast<Int>(value));
	}

	template <HasToBytes Obj>
	inline void Socket::send_connected_object(const Obj& obj)
	{
		send_connected(obj.to_bytes());
	}

	template <typename T>
	requires (HasByteData<T> || StringType<T> ||
			std::is_fundamental_v<T> || std::is_enum_v<T> ||
			ModIntType<T> ||
			HasToBytes<T> ||
			TupleLike<T>)
	inline void Socket::send_connected_value(const T& value)
	{
		if constexpr (StringType<T>)
			send_connected_str(value);
		else if constexpr (std::is_fundamental_v<T> || std::is_enum_v<T>)
			send_connected_primitive(value);
		else if constexpr (ModIntType<T>)
			send_connected_modint(value);
		else if constexpr (HasToBytes<T>)
			send_connected_object(value);
		else if constexpr (TupleLike<T>)
			send_connected_values(value);
		else
			send_connected(value);
	}

	template <TupleLike Tpl>
	inline void Socket::send_connected_values(const Tpl& values)
	{
		std::apply(
			[this](auto&... args) { (send_connected_value(args), ...); },
			values
		);
	}

	inline std::size_t Socket::recv_connected_into(HasMutableByteData auto& out)
	{
		return recv_connected_into(out.data(), out.size());
	}

	inline void Socket::recv_connected_exact_into(HasMutableByteData auto& out)
	{
		return recv_connected_exact_into(out.data(), out.size());
	}

	template <StringType Str, std::size_t chunkSize>
	inline Str Socket::recv_connected_str()
	{
		using C = typename Str::value_type;
		constexpr C nullchr = static_cast<C>(0);
		C chunk[chunkSize] = {0};
		const C* pNullChrInChunk = nullptr;
		const C* chunkEnd = chunk + chunkSize;
		std::size_t elemsRead = 0;
		bool lastChunk = false;
		Str res{};

		// while nullchr not found in chunk
		while (!lastChunk)
		{
			// get current chunk
			elemsRead = recv_connected_into(chunk, chunkSize * sizeof(C)) / sizeof(C);

			// look for null termination
			pNullChrInChunk = std::find<const C*>(chunk, chunkEnd, nullchr);

			// if has null termination, this is the last chunk
			lastChunk = (chunkEnd != pNullChrInChunk);

			// if last chunk, append until null-terination; else, append all
			if (lastChunk)
				res += chunk;
			else
				res.append(chunk, chunkSize);
		}

		const C* dataEnd = chunk + elemsRead; // end of read data in last chunk

		// res now has string, with `pNullChrInChunk` pointing to null termination
		// extra bytes are after null termination
		const std::size_t extraBytesCount = (dataEnd - pNullChrInChunk - 1) * sizeof(C);
		const byte* extraBytesStart = reinterpret_cast<const byte*>(pNullChrInChunk + 1);

		// append extra bytes to `_buffer`:
		this->_buffer.insert(this->_buffer.end(), extraBytesStart, extraBytesStart + extraBytesCount);

		return res;
	}

	template <typename T>
	requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
	inline T Socket::recv_connected_primitive()
	{
		T res{};
		recv_connected_exact_into(&res, sizeof(T));
		return res;
	}

	template <ModIntType T>
	inline T Socket::recv_connected_modint()
	{
		using Int = typename T::Int;
		return T(recv_connected_primitive<Int>());
	}

	template <HasFromBytes T>
	requires HasFixedBytesSize<T>
	inline T Socket::recv_connected_obj()
	{
		return T::from_bytes(recv_connected_exact(T::bytes_size()));
	}

	template <typename T, std::size_t chunkSize>
	requires (HasMutableByteData<T> || StringType<T> || 
			std::is_fundamental_v<T> || std::is_enum_v<T> ||
			ModIntType<T> ||
			(HasFromBytes<T> && HasFixedBytesSize<T>) ||
			TupleLike<T>)
	inline void Socket::recv_connected_value(T& out)
	{
		if constexpr (StringType<T>)
			out = recv_connected_str<T, chunkSize>();
		else if constexpr (std::is_fundamental_v<T> || std::is_enum_v<T>)
			out = recv_connected_primitive<T>();
		else if constexpr (ModIntType<T>)
			out = recv_connected_modint<T>();
		else if constexpr (HasFromBytes<T> && HasFixedBytesSize<T>)
			out = recv_connected_obj<T>();
		else if constexpr (TupleLike<T>)
			recv_connected_values<T, chunkSize>(out);
		else
			recv_connected_exact_into(out);
	}

	template <TupleLike Tpl, std::size_t chunkSize>
	inline void Socket::recv_connected_values(Tpl& values)
	{
		std::apply(
			[this](auto&... args)
			{
				(recv_connected_value<std::remove_cvref_t<decltype(args)>, chunkSize>(args), ...);
			},
			values
		);
	}

	template <IPType IP>
	inline void ConnectableSocket<IP>::close()
	{
		Base::close();
	}

	template <IPType IP>
	inline void ConnectableSocket<IP>::connect(const IP& addr, Port port)
	{
		typename IP::UnderlyingSockAddr sa{};
		addr.init_underlying(&sa, port);
		if (::connect(this->_sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
			throw SocketException("Failed to connect", Socket::get_last_sock_err());
		this->_isConnected = true;
	}

	template <IPType IP>
	inline void ConnectableSocket<IP>::bind(Port port)
	{
		this->bind(IPv4::ANY, port);
	}

	template <IPType IP>
	inline void ConnectableSocket<IP>::bind(const IP& addr, Port port)
	{
		typename IP::UnderlyingSockAddr sa{};
		addr.init_underlying(&sa, port);
		if (::bind(this->_sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
			throw SocketException("Failed to bind", Socket::get_last_sock_err());
	}

	template <IPType IP>
	inline ConnectableSocket<IP>::ConnectableSocket(Underlying sock, bool isConnected)
		: Base(sock, isConnected) { }

	template <IPType IP>
	inline TcpSocket<IP>::TcpSocket() 
		: Base(socket(IP::UNDERLYING_ADDRESS_FAMILY, SOCK_STREAM, IPPROTO_TCP)) { }
	
	template <IPType IP>
	inline TcpSocket<IP>::TcpSocket(const IP& addr, Port port)
		: Self()
	{
		this->connect(addr, port);
	}

	template <IPType IP>
	inline void TcpSocket<IP>::listen()
	{
		if (::listen(this->_sock, SOMAXCONN) < 0)
			throw SocketException("Failed to listen", Socket::get_last_sock_err());
	}

	template <IPType IP>
	inline std::pair<typename TcpSocket<IP>::Self, std::tuple<IP, Port>>
		TcpSocket<IP>::accept()
	{
		typename IP::UnderlyingSockAddr clientAddr{};
		int clientAddrLen = sizeof(clientAddr);

		auto sock = ::accept(this->_sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if (Socket::UNDERLYING_NO_SOCK == sock)
			throw SocketException("Failed to accept", Socket::get_last_sock_err());
		return std::make_pair(
			Self(sock, true), // isConnected=true
			IP::from_underlying_sock_addr(clientAddr)
		);
	}

	template <IPType IP>
	inline TcpSocket<IP>::TcpSocket(Underlying sock, bool isConnected) 
		: Base(sock, isConnected) { }

	template <IPType IP>
	inline UdpSocket<IP>::UdpSocket()
		: Base(socket(IP::UNDERLYING_ADDRESS_FAMILY, SOCK_DGRAM, IPPROTO_UDP)) { }

	template <IPType IP>
	inline void UdpSocket<IP>::disconnect()
	{
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_UNSPEC;
		if (::connect(this->_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
			throw SocketException("Failed to disconnect", Socket::get_last_sock_err());
		this->_isConnected = false;
	}

	template <IPType IP>
	inline void UdpSocket<IP>::send_to(const void* data, std::size_t size, const IP& addr, Port port)
	{
		typename IP::UnderlyingSockAddr sa{};
		addr.init_underlying(&sa, port);

		// Note: We assume here that size does not surpass int limit.
		if (static_cast<int>(size) != ::sendto(this->_sock, (const char*)data, size, 0, (struct sockaddr*)&sa, sizeof(sa)))
			throw SocketException("Failed to send", Socket::get_last_sock_err());
	}

	template <IPType IP>
	inline void UdpSocket<IP>::send_to(const HasByteData auto& data, const IP& addr, Port port)
	{
		return send_to(data.data(), data.size(), addr, port);
	}

	template <IPType IP>
	inline UdpSocket<IP>::recv_from_ret_t UdpSocket<IP>::recv_from(std::size_t maxsize)
	{
		Buffer res(maxsize, static_cast<byte>(0));
		auto ret = recv_from_into(res);
		return { Buffer(res.begin(), res.begin() + ret.count), ret.addr, ret.port };
	}

	template <IPType IP>
	inline UdpSocket<IP>::recv_from_into_ret_t UdpSocket<IP>::recv_from_into(void* out, std::size_t maxsize)
	{
		typename IP::UnderlyingSockAddr addr{};
		int addrLen = sizeof(addr);
		const int count = ::recvfrom(
			this->_sock, (char*)out, (int)maxsize, 0,
			(struct sockaddr*)&addr, &addrLen
		);
		if (count < 0)
			throw SocketException("Failed to recieve", Socket::get_last_sock_err());
		auto [ip, port] = IP::from_underlying_sock_addr(addr);
		return { static_cast<std::size_t>(count), ip, port };
	}

	template <IPType IP>
	inline UdpSocket<IP>::recv_from_into_ret_t UdpSocket<IP>::recv_from_into(HasMutableByteData auto& out)
	{
		return recv_from_into(out.data(), out.size());
	}

	template <IPType IP>
	inline UdpSocket<IP>::UdpSocket(Underlying sock, bool isConnected)
		: Base(sock, isConnected) { }
}

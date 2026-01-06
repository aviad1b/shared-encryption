/*********************************************************************
 * \file   Socket_impl.hpp
 * \brief  Template implementation of socket-related classes.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Socket.hpp"

#ifdef SENC_WINDOWS
#include <ws2tcpip.h>
#endif

#include <algorithm>

namespace senc::utils
{
	inline void Socket::send_connected(const HasByteData auto& data)
	{
		// Note: We assume here that data.size() does not surpass int limit.
		return send_connected(data.data(), data.size());
	}

	template <std::endian endianess>
	inline void Socket::send_connected_str(const StringType auto& data)
	{
		using Str = std::remove_cvref_t<decltype(data)>;
		using C = typename Str::value_type;

		// if endianess same as native (or elem size is one meaning no need to reverse)
		if constexpr (std::endian::native == endianess)
			return send_connected(data.c_str(), (data.size() + 1) * sizeof(C));

		// otherwise, reverse each elem then send
		Str copy = data;
		for (C& c : copy)
			std::reverse(reinterpret_cast<byte*>(&c), reinterpret_cast<byte*>(&c + 1));
		return send_connected(copy.c_str(), (copy.size() + 1) * sizeof(C));
	}

	template <std::endian endianess>
	inline void Socket::send_connected_primitive(auto value)
	requires (std::is_fundamental_v<std::remove_cvref_t<decltype(value)>> || 
		std::is_enum_v<std::remove_cvref_t<decltype(value)>>)
	{
		using T = std::remove_cvref_t<decltype(value)>;

		// if endianess same as native, simply send
		if constexpr (std::endian::native == endianess)
			send_connected(&value, sizeof(value));
		else // if endianess diff from native, reverse and then send
		{
			T copy = value;
			std::reverse(reinterpret_cast<byte*>(&copy), reinterpret_cast<byte*>(&copy + 1));
			send_connected(&copy, sizeof(copy));
		}
	}

	template <std::endian endianess>
	inline void Socket::send_connected_modint(const ModIntType auto& value)
	{
		using T = std::remove_cvref_t<decltype(value)>;
		using Int = typename T::Int;
		send_connected_primitive<endianess>(static_cast<Int>(value));
	}

	template <HasToBytes Obj>
	inline void Socket::send_connected_object(const Obj& obj)
	{
		send_connected(obj.to_bytes());
	}

	template <std::endian endianess>
	inline void Socket::send_connected_value(const auto& value)
	requires (HasByteData<std::remove_cvref_t<decltype(value)>> ||
		StringType<std::remove_cvref_t<decltype(value)>> ||
		std::is_fundamental_v<std::remove_cvref_t<decltype(value)>> ||
		std::is_enum_v<std::remove_cvref_t<decltype(value)>> ||
		ModIntType<std::remove_cvref_t<decltype(value)>> ||
		HasToBytes<std::remove_cvref_t<decltype(value)>> ||
		TupleLike<std::remove_cvref_t<decltype(value)>>)
	{
		using T = std::remove_cvref_t<decltype(value)>;
		if constexpr (StringType<T>)
			send_connected_str<endianess>(value);
		else if constexpr (std::is_fundamental_v<T> || std::is_enum_v<T>)
			send_connected_primitive<endianess>(value);
		else if constexpr (ModIntType<T>)
			send_connected_modint<endianess>(value);
		else if constexpr (HasToBytes<T>)
			send_connected_object(value);
		else if constexpr (TupleLike<T>)
			send_connected_values<endianess>(value);
		else
			send_connected(value);
	}

	template <std::endian endianess>
	inline void Socket::send_connected_values(const TupleLike auto& values)
	{
		std::apply(
			[this](auto&... args) { (this->send_connected_value<endianess>(args), ...); },
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

	template <StringType Str, std::endian endianess, std::size_t chunkSize>
	inline Str Socket::recv_connected_str()
	{
		using C = typename Str::value_type;
		constexpr C nullchr = static_cast<C>(0);
		C chunk[chunkSize] = {0};
		const C* pNullChrInChunk = nullptr;
		const C* chunkEnd = chunk + chunkSize;
		std::size_t bytesRead = 0;
		bool lastChunk = false;
		Str res{};

		// while nullchr not found in chunk
		while (!lastChunk)
		{
			// get current chunk
			bytesRead = recv_connected_into(chunk, chunkSize * sizeof(C));

			// look for null termination
			pNullChrInChunk = std::find<const C*>(chunk, chunkEnd, nullchr);

			// if has null termination, this is the last chunk
			lastChunk = (chunkEnd != pNullChrInChunk);

			// if last chunk, append until null-terination; else, append all
			if (lastChunk)
				res.append(static_cast<const C*>(chunk), pNullChrInChunk);
			else
				res.append(chunk, chunkSize);
		}

		// end of read data in last chunk
		const byte* dataEnd = 
			reinterpret_cast<const byte*>(static_cast<const C*>(chunk)) + bytesRead;

		// res now has string, with `pNullChrInChunk` pointing to null termination
		// extra bytes are after null termination
		const byte* extraBytesStart = reinterpret_cast<const byte*>(pNullChrInChunk + 1);

		// append extra bytes to `_buffer`:
		this->_buffer.insert(this->_buffer.end(), extraBytesStart, dataEnd);

		// if required endianess is not same as native, reverse each elem
		if constexpr (std::endian::native != endianess)
			for (C& c : res)
				std::reverse(reinterpret_cast<byte*>(&c), reinterpret_cast<byte*>(&c + 1));

		return res;
	}

	template <typename T, std::endian endianess>
	requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
	inline T Socket::recv_connected_primitive()
	{
		T res{};
		recv_connected_exact_into(&res, sizeof(T));

		// if endianess not native, reverse
		if constexpr (std::endian::native != endianess)
			std::reverse(reinterpret_cast<byte*>(&res), reinterpret_cast<byte*>(&res + 1));

		return res;
	}

	template <ModIntType T, std::endian endianess>
	inline T Socket::recv_connected_modint()
	{
		using Int = typename T::Int;
		return T(recv_connected_primitive<Int, endianess>());
	}

	template <HasFromBytes T>
	requires HasFixedBytesSize<T>
	inline T Socket::recv_connected_obj()
	{
		return T::from_bytes(recv_connected_exact(T::bytes_size()));
	}

	template <typename T, std::endian endianess, std::size_t chunkSize>
	requires (HasMutableByteData<T> || StringType<T> || 
			std::is_fundamental_v<T> || std::is_enum_v<T> ||
			ModIntType<T> ||
			(HasFromBytes<T> && HasFixedBytesSize<T>) ||
			TupleLike<T>)
	inline void Socket::recv_connected_value(T& out)
	{
		if constexpr (StringType<T>)
			out = recv_connected_str<T, endianess, chunkSize>();
		else if constexpr (std::is_fundamental_v<T> || std::is_enum_v<T>)
			out = recv_connected_primitive<T, endianess>();
		else if constexpr (ModIntType<T>)
			out = recv_connected_modint<T, endianess>();
		else if constexpr (HasFromBytes<T> && HasFixedBytesSize<T>)
			out = recv_connected_obj<T, endianess>();
		else if constexpr (TupleLike<T>)
			recv_connected_values<T, endianess, chunkSize>(out);
		else
			recv_connected_exact_into(out);
	}

	template <TupleLike Tpl, std::endian endianess, std::size_t chunkSize>
	inline void Socket::recv_connected_values(Tpl& values)
	{
		std::apply(
			[this](auto&... args)
			{
				(this->recv_connected_value<
					std::remove_cvref_t<decltype(args)>,
					endianess,
					chunkSize>(args), ...);
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
			throw SocketException("Failed to connect", SocketUtils::get_last_sock_err());
		this->_isConnected = true;
	}

	template <IPType IP>
	inline void ConnectableSocket<IP>::bind(Port port)
	{
		this->bind(IP::any(), port);
	}

	template <IPType IP>
	inline void ConnectableSocket<IP>::bind(const IP& addr, Port port)
	{
		typename IP::UnderlyingSockAddr sa{};
		addr.init_underlying(&sa, port);
		if (::bind(this->_sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
			throw SocketException("Failed to bind", SocketUtils::get_last_sock_err());
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
			throw SocketException("Failed to listen", SocketUtils::get_last_sock_err());
	}

	template <IPType IP>
	inline std::pair<typename TcpSocket<IP>::Self, std::tuple<IP, Port>>
		TcpSocket<IP>::accept()
	{
		typename IP::UnderlyingSockAddr clientAddr{};
		socklen_t clientAddrLen = sizeof(clientAddr);

		auto sock = ::accept(this->_sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if (Socket::UNDERLYING_NO_SOCK == sock)
			throw SocketException("Failed to accept", SocketUtils::get_last_sock_err());
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
		struct sockaddr_in addr{};
		addr.sin_family = AF_UNSPEC;
		if (::connect(this->_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
			throw SocketException("Failed to disconnect", SocketUtils::get_last_sock_err());
		this->_isConnected = false;
	}

	template <IPType IP>
	inline void UdpSocket<IP>::send_to(const void* data, std::size_t size, const IP& addr, Port port)
	{
		typename IP::UnderlyingSockAddr sa{};
		addr.init_underlying(&sa, port);

		// Note: We assume here that size does not surpass int limit.
		if (static_cast<int>(size) != ::sendto(this->_sock, (const char*)data, size, 0, (struct sockaddr*)&sa, sizeof(sa)))
			throw SocketException("Failed to send", SocketUtils::get_last_sock_err());
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
		socklen_t addrLen = sizeof(addr);
		const int count = ::recvfrom(
			this->_sock, (char*)out, (int)maxsize, 0,
			(struct sockaddr*)&addr, &addrLen
		);
		if (count < 0)
			throw SocketException("Failed to recieve", SocketUtils::get_last_sock_err());
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

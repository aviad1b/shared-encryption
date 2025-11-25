/*********************************************************************
 * \file   Socket_impl.hpp
 * \brief  Template implementation of socket-related classes.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Socket.hpp"

namespace senc::utils
{
	inline void Socket::send_connected(const HasByteData auto& data)
	{
		// Note: We assume here that data.size() does not surpass int limit.
		return send_connected(data.data(), data.size());
	}

	inline void Socket::send_connected_str(const StringType auto& data)
	{
		using C = typename std::remove_cvref_t<decltype(data)>::value_type;
		return send_connected(data.c_str(), (data.size + 1) * sizeof(C));
	}

	inline std::size_t Socket::recv_connected_into(HasMutableByteData auto& out)
	{
		return recv_connected_into(out.data(), out.size());
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
	inline TcpSocket<IP>::Self TcpSocket<IP>::accept()
	{
		auto sock = ::accept(this->_sock, nullptr, nullptr);
		if (Socket::UNDERLYING_NO_SOCK == sock)
			throw SocketException("Failed to accept", Socket::get_last_sock_err());
		return Self(sock, true); // isConnected=true
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

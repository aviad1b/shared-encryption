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
	template <IPType IP>
	inline bool ConnectableSocket<IP>::is_connected() const
	{
		return this->_isConnected;
	}

	template <IPType IP>
	inline void ConnectableSocket<IP>::connect(const IP& addr, Port port)
	{
		typename IP::UnderlyingSockAddr sa{};
		addr.init_underlying(&sa, port);
		if (::connect(this->_sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
			throw SocketException("Failed to connect", get_last_sock_err());
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
			throw SocketException("Failed to bind", get_last_sock_err());
	}

	template <IPType IP>
	inline ConnectableSocket<IP>::ConnectableSocket(Underlying sock)
		: Base(sock) { }

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
			throw SocketException("Failed to listen", get_last_sock_err());
	}

	template <IPType IP>
	inline TcpSocket<IP>::Self TcpSocket<IP>::accept()
	{
		auto sock = ::accept(this->_sock, nullptr, nullptr);
		if (Socket::UNDERLYING_NO_SOCK == sock)
			throw SocketException("Failed to accept", get_last_sock_err());
		return sock;
	}

	template <IPType IP>
	inline UdpSocket<IP>::UdpSocket()
		: Base(socket(IP::UNDERLYING_ADDRESS_FAMILY, SOCK_DGRAM, IPPROTO_UDP)) { }

	template <IPType IP>
	inline void UdpSocket<IP>::disconnect()
	{
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_UNSPEC;
		if (::connect(this->_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
			throw SocketException("Failed to disconnect", get_last_sock_err());
	}

	template <IPType IP>
	inline void UdpSocket<IP>::sendto(const std::vector<std::byte>& data, const IP& addr, Port port)
	{
		typename IP::UnderlyingSockAddr sa{};
		addr.init_underlying(&sa, port);

		// Note: We assume here that data.size() does not surpass int limit.
		if (static_cast<int>(data.size()) != ::sendto(this->_sock, (const char*)data.data(), data.size(), 0, (struct sockaddr*)&sa, sizeof(sa)))
			throw SocketException("Failed to send", get_last_sock_err());
	}

	template <IPType IP>
	inline std::vector<std::byte> UdpSocket<IP>::recvfrom(std::size_t maxsize)
	{
		std::vector<std::byte> res(maxsize, static_cast<std::byte>(0));
		const int count = ::recvfrom(this->_sock, (char*)res.data(), maxsize, 0, nullptr, nullptr);
		if (count < 0)
			throw SocketException("Failed to recieve", get_last_sock_err());
		return std::vector<std::byte>(res.begin(), res.begin() + count);
	}
}

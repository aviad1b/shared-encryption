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
		using Underlying = typename std::remove_cvref_t<decltype(addr)>::Underlying;
		Underlying sa;
		addr.init_underlying(&sa, port);
		if (::connect(this->_sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
			throw SocketException("Failed to connect");
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
		using Underlying = typename std::remove_cvref_t<decltype(addr)>::Underlying;
		Underlying sa;
		addr.init_underlying(&sa, port);
		if (::bind(this->_sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
			throw SocketException("Failed to bind");
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
			throw SocketException("Failed to listen");
	}

	template <IPType IP>
	inline TcpSocket<IP>::Self TcpSocket<IP>::accept()
	{
		auto sock = ::accept(this->_sock, nullptr, nullptr);
		if (Socket::UNDERLYING_NO_SOCK == sock)
			throw SocketException("Failed to accept");
		return sock;
	}
}

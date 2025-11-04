#include "Socket.hpp"

namespace senc::utils
{
	inline void ConnectableSocket::connect(const IPType auto& addr, Port port)
	{
		using Underlying = typename std::remove_cvref_t<decltype(addr)>::Underlying;
		Underlying sa;
		addr.init_underlying(&sa, port);
		if (::connect(this->_sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
			throw SocketException("Failed to connect");
		this->_isConnected = true;
	}

	inline void ConnectableSocket::bind(const IPType auto& addr, Port port)
	{
		using Underlying = typename std::remove_cvref_t<decltype(addr)>::Underlying;
		Underlying sa;
		addr.init_underlying(&sa, port);
		if (::bind(this->_sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
			throw SocketException("Failed to bind");
	}
}

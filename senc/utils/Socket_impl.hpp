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
}

#include "Socket.hpp"

#include <ws2tcpip.h>

namespace senc::utils
{
	IPv4::IPv4(const std::string& addr) : _addr(addr) { }

	IPv4::IPv4(std::string&& addr) : _addr(std::move(addr)) { }

	bool IPv4::operator==(const Self& other) const noexcept
	{
		return this->_addr == other._addr;
	}

	const std::string& IPv4::as_str() const noexcept
	{
		return this->_addr;
	}

	void IPv4::init_underlying(Underlying* out, Port port) const noexcept
	{
		out->sin_port = htons(port);
		out->sin_family = AF_INET; // IPv4
		out->sin_addr.s_addr = *this == ANY ? INADDR_ANY
			: inet_addr(this->as_str().c_str());
	}

	IPv6::IPv6(const std::string& addr) : _addr(addr) { }

	IPv6::IPv6(std::string&& addr) : _addr(std::move(addr)) { }

	bool IPv6::operator==(const Self& other) const noexcept
	{
		return this->_addr == other._addr;
	}

	const std::string& IPv6::as_str() const noexcept
	{
		return this->_addr;
	}

	void IPv6::init_underlying(Underlying* out, Port port) const
	{
		out->sin6_port = htons(port);
		out->sin6_family = AF_INET6; // IPv6
		if (1 != inet_pton(AF_INET6, this->as_str().c_str(), &out->sin6_addr))
			throw SocketException("Failed to parse IPv6 address");
	}

	SocketException::SocketException(const std::string& msg) : Base(msg) { }

	SocketException::SocketException(std::string&& msg) : Base(std::move(msg)) { }
	
	Socket::~Socket()
	{
		this->close();
	}

	void Socket::send(const std::vector<std::byte>& data)
	{
		// Note: We assume here that data.size() does not surpass int limit.
		if (static_cast<int>(data.size()) != ::send(this->_sock, (const char*)data.data(), data.size(), 0))
			throw SocketException("Failed to send");
	}

	std::vector<std::byte> Socket::recv(std::size_t maxsize)
	{
		std::vector<std::byte> res(maxsize, static_cast<std::byte>(0));
		const int count = ::recv(this->_sock, (char*)res.data(), maxsize, 0);
		if (count < 0)
			throw SocketException("Failed to recieve");
		return std::vector<std::byte>(res.begin(), res.begin() + count);
	}

	Socket::Socket(int underlyingAddressFamily, int underlyingType, int underlyingProtocol)
		: _sock(socket(underlyingAddressFamily, underlyingType, underlyingProtocol))
	{
		if (UNDERLYING_NO_SOCK == this->_sock)
			throw SocketException("Failed to create socket");
	}

	void Socket::close()
	{
		try { ::closesocket(this->_sock); }
		catch (...) { }
	}
}

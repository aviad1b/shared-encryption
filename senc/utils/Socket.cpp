/*********************************************************************
 * \file   Socket.cpp
 * \brief  Implementation of socket-related classes.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Socket.hpp"

#include <ws2tcpip.h>
#include <cstring>

namespace senc::utils
{
	const IPv4::Self IPv4::ANY("0.0.0.0");

	IPv4::IPv4(const char* addr) : Self(std::string(addr)) { }

	IPv4::IPv4(const std::string& addr) : Self(std::string(addr)) { }

	IPv4::IPv4(std::string&& addr) : _addrStr(std::move(addr))
	{
		if (1 != inet_pton(AF_INET, this->_addrStr.c_str(), &this->_addr))
			throw SocketException("Invalid IPv4 address: " + this->_addrStr);
	}

	bool IPv4::operator==(const Self& other) const noexcept
	{
		return this->_addrStr == other._addrStr;
	}

	const std::string& IPv4::as_str() const noexcept
	{
		return this->_addrStr;
	}

	void IPv4::init_underlying(UnderlyingSockAddr* out, Port port) const noexcept
	{
		out->sin_port = htons(port);
		out->sin_family = AF_INET; // IPv4
		out->sin_addr = this->_addr;
	}

	const IPv6::Self IPv6::ANY("::");

	IPv6::IPv6(const char* addr) : Self(std::string(addr)) { }

	IPv6::IPv6(const std::string& addr) : Self(std::string(addr)) { }

	IPv6::IPv6(std::string&& addr) : _addrStr(std::move(addr))
	{
		if (1 != inet_pton(AF_INET6, this->_addrStr.c_str(), &this->_addr))
			throw SocketException("Invalid IPv6 address: " + this->_addrStr);
	}

	bool IPv6::operator==(const Self& other) const noexcept
	{
		return this->_addrStr == other._addrStr;
	}

	const std::string& IPv6::as_str() const noexcept
	{
		return this->_addrStr;
	}

	void IPv6::init_underlying(UnderlyingSockAddr* out, Port port) const noexcept
	{
		out->sin6_port = htons(port);
		out->sin6_family = AF_INET6; // IPv6
		out->sin6_addr = this->_addr;
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

	Socket::Socket(Underlying sock) : _sock(sock)
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

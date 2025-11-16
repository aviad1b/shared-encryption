/*********************************************************************
 * \file   Socket.cpp
 * \brief  Implementation of socket-related classes.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Socket.hpp"

#include <experimental/scope>
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

	SocketException::SocketException(const std::string& msg, const std::string& info)
		: Base(msg, info) { }

	SocketException::SocketException(std::string&& msg, const std::string& info)
		: Base(std::move(msg), info) { }

	SocketInitializer::~SocketInitializer()
	{
		try { WSACleanup(); }
		catch (...) { }
	}

	SocketInitializer::SocketInitializer()
	{
		WSADATA wsa_data{};
		if (0 != WSAStartup(MAKEWORD(2, 2), &wsa_data))
			throw SocketException("WSAStartup failed", Socket::get_last_sock_err());
	}
	
	Socket::~Socket()
	{
		this->close();
	}

	bool Socket::is_closed() const
	{
		return UNDERLYING_NO_SOCK != this->_sock;
	}

	void Socket::send(const Buffer& data)
	{
		// Note: We assume here that data.size() does not surpass int limit.
		if (static_cast<int>(data.size()) != ::send(this->_sock, (const char*)data.data(), data.size(), 0))
			throw SocketException("Failed to send", get_last_sock_err());
	}

	Buffer Socket::recv(std::size_t maxsize)
	{
		Buffer res(maxsize, static_cast<byte>(0));
		const int count = ::recv(this->_sock, (char*)res.data(), maxsize, 0);
		if (count < 0)
			throw SocketException("Failed to recieve", get_last_sock_err());
		return Buffer(res.begin(), res.begin() + count);
	}

	Socket::Socket(Underlying sock) : _sock(sock)
	{
		if (UNDERLYING_NO_SOCK == this->_sock)
			throw SocketException("Failed to create socket", get_last_sock_err());
	}

	Socket::Socket(Self&& other) : _sock(other._sock)
	{
		other._sock = UNDERLYING_NO_SOCK;
	}

	Socket::Self& Socket::operator=(Self&& other)
	{
		this->close();
		this->_sock = other._sock;
		other._sock = UNDERLYING_NO_SOCK;
		return *this;
	}

	void Socket::close()
	{
		try { ::closesocket(this->_sock); }
		catch (...) { }
		this->_sock = UNDERLYING_NO_SOCK;
	}

	std::string Socket::get_last_sock_err()
	{
		DWORD err = WSAGetLastError();

		LPSTR msg = nullptr;
		auto guard = std::experimental::scope_exit{
			[&msg]{ if (msg) LocalFree(msg); }
		};

		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			err,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPSTR)&msg,
			0,
			nullptr
		);

		return msg ? std::string(msg) : "";
	}

	const SocketInitializer Socket::SOCKET_INITIALIZER;
}

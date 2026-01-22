/*********************************************************************
 * \file   Socket.cpp
 * \brief  Implementation of socket-related classes.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Socket.hpp"

#include "env.hpp"
#ifdef SENC_WINDOWS
#include "../utils/AtScopeExit.hpp"
#else
#include <signal.h>
#include <poll.h>
#endif

#include <cstring>

namespace senc::utils
{
	const SocketInitializer SocketInitializer::SOCKET_INITIALIZER;

#ifdef SENC_WINDOWS
	std::string SocketUtils::get_last_sock_err()
	{
		DWORD err = WSAGetLastError();

		LPSTR msg = nullptr;
		const auto guard = AtScopeExit{
			[&msg] { if (msg) LocalFree(msg); }
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

		std::string res = msg ? std::string(msg) : "";
		if (res.ends_with("\n"))
			res = res.substr(0, res.length() - 1);
		return res;
	}
#else
	std::string SocketUtils::get_last_sock_err()
	{
		static constexpr std::size_t MAX_MSG_LEN = 256;
		char msg[MAX_MSG_LEN] = "";

		strerror_r(errno, msg, MAX_MSG_LEN);

		std::string res = msg;
		if (res.ends_with("\n"))
			res = res.substr(0, res.length() - 1);
		return res;
	}
#endif

#ifdef SENC_WINDOWS
	SocketInitializer::~SocketInitializer()
	{
		try { WSACleanup(); }
		catch (...) { }
	}
#else
	SocketInitializer::~SocketInitializer()
	{
		// nothing to do
	}
#endif

#ifdef SENC_WINDOWS
	SocketInitializer::SocketInitializer()
	{
		WSADATA wsa_data{};
		if (0 != WSAStartup(MAKEWORD(2, 2), &wsa_data))
			throw SocketException("WSAStartup failed", SocketUtils::get_last_sock_err());
	}
#else
	SocketInitializer::SocketInitializer()
	{
		signal(SIGPIPE, SIG_IGN); // make send() return error instead of signal
	}
#endif
	
	Socket::~Socket()
	{
		this->close();
	}

	bool Socket::is_closed() const
	{
		return UNDERLYING_NO_SOCK != this->_sock;
	}

	bool Socket::is_connected() const
	{
		return this->_isConnected;
	}

	void Socket::send_connected(const void* data, std::size_t size)
	{
		if (!is_connected())
			throw SocketException("Failed to send", "Socket is not connected");

		// Note: We assume here that size does not surpass int limit.
		if (static_cast<int>(size) != ::send(this->_sock, (const char*)data, (int)size, 0))
			throw SocketException("Failed to send", SocketUtils::get_last_sock_err());
	}

	Buffer Socket::recv_connected(std::size_t maxsize)
	{
		Buffer res(maxsize, static_cast<byte>(0));
		const std::size_t count = recv_connected_into(res);
		return Buffer(res.begin(), res.begin() + count);
	}

	Buffer Socket::recv_connected_exact(std::size_t size)
	{
		Buffer res(size, static_cast<byte>(0));
		recv_connected_exact_into(res);
		return res;
	}

	std::size_t Socket::recv_connected_into(void* out, std::size_t maxsize)
	{
		// if has leftover data, consider connected and output leftover data first
		std::size_t leftoverBytes = out_leftover_data(out, maxsize);
		if (!(leftoverBytes > 0 || is_connected()))
			throw SocketException("Failed to recieve", "Socket is not connected");

		if (leftoverBytes > 0 && !underlying_has_data(this->_sock))
			return leftoverBytes; // if read leftover, and has nothing more - stop here

		const int count = ::recv(this->_sock, (char*)out + leftoverBytes, (int)(maxsize - leftoverBytes), 0);
		if (count < 0)
			throw SocketException("Failed to recieve", SocketUtils::get_last_sock_err());
		return count + leftoverBytes;
	}

	void Socket::recv_connected_exact_into(void* out, std::size_t size)
	{
		std::size_t bytesRead = 0;
		while (bytesRead < size)
		{
			bytesRead += recv_connected_into(
				reinterpret_cast<byte*>(out) + bytesRead,
				size - bytesRead
			);
		}
	}

	Socket::Socket(Underlying sock, bool isConnected)
		: _sock(sock), _isConnected(isConnected)
	{
		if (UNDERLYING_NO_SOCK == this->_sock)
			throw SocketException("Failed to create socket", SocketUtils::get_last_sock_err());
	}

	Socket::Socket(Self&& other) : _sock(other._sock), _isConnected(other._isConnected)
	{
		other._sock = UNDERLYING_NO_SOCK;
		other._isConnected = false;
	}

	Socket::Self& Socket::operator=(Self&& other)
	{
		this->close();
		this->_sock = other._sock;
		this->_isConnected = other._isConnected;
		other._sock = UNDERLYING_NO_SOCK;
		other._isConnected = false;
		return *this;
	}

	void Socket::close()
	{
		if (UNDERLYING_NO_SOCK == this->_sock)
			return;
		try
		{
#ifdef SENC_WINDOWS
			::closesocket(this->_sock);
#else
			::close(this->_sock);
#endif
		}
		catch (...) { }
		this->_sock = UNDERLYING_NO_SOCK;
		this->_isConnected = false;
	}

	std::size_t Socket::out_leftover_data(void* out, std::size_t maxsize)
	{
		if (this->_buffer.empty())
			return 0; // no leftover output

		const std::size_t outputSize = std::min(this->_buffer.size(), maxsize);
		std::memcpy(out, this->_buffer.data(), outputSize);
		this->_buffer = Buffer(this->_buffer.begin() + outputSize, this->_buffer.end());
		return outputSize;
	}

#ifdef SENC_WINDOWS
	bool Socket::underlying_has_data(Underlying sock)
	{
		fd_set rfds{};
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);

		struct timeval tv{};
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		int r = select(0, &rfds, nullptr, nullptr, &tv);
		if (r < 0)
			throw SocketException("Failed to recieve", SocketUtils::get_last_sock_err());
		return r != 0; // true if data is available
	}
#else
	bool Socket::underlying_has_data(Underlying sock)
	{
		pollfd pfd{};
		pfd.fd = sock;
		pfd.events = POLLIN;

		int r = poll(&pfd, 1, 0); // timeout = 0 -> poll

		if (r < 0)
			throw SocketException("Failed to receive", SocketUtils::get_last_sock_err());

		return (pfd.revents & POLLIN) != 0;
	}
#endif
}

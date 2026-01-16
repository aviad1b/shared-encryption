/*********************************************************************
 * \file   ip.cpp
 * \brief  Implementation of IP-related utilities.
 * 
 * \author aviad1b
 * \date   January 2026, 5786
 *********************************************************************/

#include "ip.hpp"

#include "env.hpp"
#ifdef SENC_WINDOWS
#include <ws2tcpip.h>
#endif

namespace senc::utils
{
	const IPv4::Self& IPv4::any()
	{
		static const Self ANY("0.0.0.0");
		return ANY;
	}

	const IPv4::Self& IPv4::loopback()
	{
		static const Self& LOOPBACK("127.0.0.1");
		return LOOPBACK;
	}

	IPv4::IPv4(const Underlying& underlying) : _addr(underlying)
	{
		char addrStr[INET_ADDRSTRLEN] = "";
		if (!::inet_ntop(AF_INET, &underlying, addrStr, INET_ADDRSTRLEN))
			throw IPException("Unknown IPv4 address");
		this->_addrStr = addrStr;
	}

	std::tuple<IPv4::Self, Port> IPv4::from_underlying_sock_addr(
		const UnderlyingSockAddr& underlyingSockAddr)
	{
		return { Self(underlyingSockAddr.sin_addr), underlyingSockAddr.sin_port };
	}

	IPv4::IPv4(const char* addr) : Self(std::string(addr)) { }

	IPv4::IPv4(const std::string& addr) : Self(std::string(addr)) { }

	IPv4::IPv4(std::string&& addr) : _addrStr(std::move(addr))
	{
		if (1 != inet_pton(AF_INET, this->_addrStr.c_str(), &this->_addr))
			throw IPException("Invalid IPv4 address: " + this->_addrStr);
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

	const IPv6::Self& IPv6::any()
	{
		static const Self ANY("::");
		return ANY;
	}

	const IPv6::Self& IPv6::loopback()
	{
		static const Self& LOOPBACK("::1");
		return LOOPBACK;
	}

	IPv6::IPv6(const Underlying& underlying) : _addr(underlying)
	{
		char addrStr[INET6_ADDRSTRLEN] = "";
		if (!::inet_ntop(AF_INET6, &underlying, addrStr, INET6_ADDRSTRLEN))
			throw IPException("Unknown IPv6 address");
		this->_addrStr = addrStr;
	}

	std::tuple<IPv6::Self, Port> IPv6::from_underlying_sock_addr(
		const UnderlyingSockAddr& underlyingSockAddr)
	{
		return { Self(underlyingSockAddr.sin6_addr), underlyingSockAddr.sin6_port };
	}

	IPv6::IPv6(const char* addr) : Self(std::string(addr)) { }

	IPv6::IPv6(const std::string& addr) : Self(std::string(addr)) { }

	IPv6::IPv6(std::string&& addr) : _addrStr(std::move(addr))
	{
		if (1 != inet_pton(AF_INET6, this->_addrStr.c_str(), &this->_addr))
			throw IPException("Invalid IPv6 address: " + this->_addrStr);
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
}

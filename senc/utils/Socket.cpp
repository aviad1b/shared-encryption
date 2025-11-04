#include "Socket.hpp"

namespace senc::utils
{
	IPv4::IPv4(const std::string& addr) : _addr(addr) { }

	IPv4::IPv4(std::string&& addr) : _addr(std::move(addr)) { }

	bool IPv4::operator==(const Self& other) const
	{
		return this->_addr == other._addr;
	}

	IPv6::IPv6(const std::string& addr) : _addr(addr) { }

	IPv6::IPv6(std::string&& addr) : _addr(std::move(addr)) { }

	bool IPv6::operator==(const Self& other) const
	{
		return this->_addr == other._addr;
	}
}

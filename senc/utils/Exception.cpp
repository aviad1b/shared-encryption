#include "Exception.hpp"

namespace senc::utils
{
	Exception::Exception(const std::string& msg) : _msg(msg) { }

	Exception::Exception(std::string&& msg) : _msg(std::move(msg)) { }
	
	const char* Exception::what() const noexcept
	{
		return this->_msg.c_str();
	}
}

#include "ModInt.hpp"
/*********************************************************************
 * \file   ModInt.cpp
 * \brief  Contains implementation of modular int utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

namespace senc::utils
{
	ModException::ModException(const std::string& msg) : Base(msg) { }
	
	ModException::ModException(std::string&& msg) : Base(std::move(msg)) { }
	
	ModException::ModException(const std::string& msg, const std::string& info)
		: Base(msg, info) { }

	ModException::ModException(std::string&& msg, const std::string& info)
		: Base(std::move(msg), info) { }
}

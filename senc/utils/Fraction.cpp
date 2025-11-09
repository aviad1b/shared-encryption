/*********************************************************************
 * \file   Fraction.cpp
 * \brief  Contains definition of fraction utils.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "Fraction.hpp"

namespace senc::utils
{
	FractionException::FractionException(const std::string& msg)
		: Base(msg) { }

	FractionException::FractionException(std::string&& msg)
		: Base(std::move(msg)) { }

	FractionException::FractionException(const std::string& msg, const std::string& info)
		: Base(msg, info) { }

	FractionException::FractionException(std::string&& msg, const std::string& info)
		: Base(std::move(msg), info) { }
}

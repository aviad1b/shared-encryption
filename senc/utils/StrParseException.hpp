/*********************************************************************
 * \file   StrParseException.hpp
 * \brief  Header of StrParseException class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "Exception.hpp"

namespace senc::utils
{
	/**
	 * @class senc::utils::StrParseException
	 * @brief Type of exception thrown on string parse errors.
	 */
	class StrParseException : public Exception
	{
	public:
		using Self = StrParseException;
		using Base = Exception;

		StrParseException(const std::string& msg);

		StrParseException(std::string&& msg);

		StrParseException(const std::string& msg, const std::string& info);

		StrParseException(std::string&& msg, const std::string& info);

		StrParseException(const Self&) = default;

		Self& operator=(const Self&) = default;

		StrParseException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};
}

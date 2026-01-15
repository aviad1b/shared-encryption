/*********************************************************************
 * \file   ConnEstablishException.hpp
 * \brief  Contains ConnEstablishException class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/Exception.hpp"

namespace senc
{
	/**
	 * @class senc::ConnEstablishException
	 * @brief Type of exceptions thrown on Server errors.
	 */
	class ConnEstablishException : public utils::Exception
	{
	public:
		using Self = ConnEstablishException;
		using Base = utils::Exception;

		ConnEstablishException(const std::string& msg) : Base(msg) { }

		ConnEstablishException(std::string&& msg) : Base(std::move(msg)) { }

		ConnEstablishException(const std::string& msg, const std::string& info) : Base(msg, info) { }

		ConnEstablishException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) { }

		ConnEstablishException(const Self&) = default;

		Self& operator=(const Self&) = default;

		ConnEstablishException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};
}

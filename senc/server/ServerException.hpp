/*********************************************************************
 * \file   ServerException.hpp
 * \brief  Contains ServerException class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../utils/Exception.hpp"

namespace senc::server
{
	/**
 * @class senc::server::ServerException
 * @brief Type of exceptions thrown on Server errors.
 */
	class ServerException : public utils::Exception
	{
	public:
		using Self = ServerException;
		using Base = utils::Exception;

		ServerException(const std::string& msg) : Base(msg) { }

		ServerException(std::string&& msg) : Base(std::move(msg)) { }

		ServerException(const std::string& msg, const std::string& info) : Base(msg, info) { }

		ServerException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) { }

		ServerException(const Self&) = default;

		Self& operator=(const Self&) = default;

		ServerException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};
}

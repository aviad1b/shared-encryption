/*********************************************************************
 * \file   UserExistsException.hpp
 * \brief  Contains UserExistsException class.
 *
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../utils/Exception.hpp"

namespace senc::server
{
	/**
	 * @class senc::server::UserExistsException
	 * @brief Type of exceptions thrown by server storage when 
	 *        trying to make user that already exists.
	 */
	class UserExistsException : public utils::Exception
	{
	public:
		using Self = UserExistsException;
		using Base = utils::Exception;

		UserExistsException(const std::string& msg) : Base(msg) {}

		UserExistsException(std::string&& msg) : Base(std::move(msg)) {}

		UserExistsException(const std::string& msg, const std::string& info) : Base(msg, info) {}

		UserExistsException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) {}

		UserExistsException(const Self&) = default;

		Self& operator=(const Self&) = default;

		UserExistsException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};
}

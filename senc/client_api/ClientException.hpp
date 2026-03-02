/*********************************************************************
 * \file   ClientException.hpp
 * \brief  Contains `ClientException` class.
 *
 * \author aviad1b
 * \date   December 2025, Adar 5786
 *********************************************************************/

#pragma once

#include "../utils/Exception.hpp"

namespace senc::clientapi
{
	/**
	 * @class senc::clientapi::ClientException
	 * @brief Type of exceptions thrown on Client errors.
	 */
	class ClientException : public utils::Exception
	{
	public:
		using Self = ClientException;
		using Base = utils::Exception;

		ClientException(const std::string& msg) : Base(msg) { }

		ClientException(std::string&& msg) : Base(std::move(msg)) { }

		ClientException(const std::string& msg, const std::string& info) : Base(msg, info) { }

		ClientException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) { }

		ClientException(const Self&) = default;

		Self& operator=(const Self&) = default;

		ClientException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};
}

/*********************************************************************
 * \file   file_exceptions.hpp
 * \brief  Contains definitions of file-related exceptions.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "Exception.hpp"

namespace senc::utils
{
	/**
	 * @class senc::utils::FileException
	 * @brief Type of exception thrown on file errors.
	 */
	class FileException : public Exception
	{
	public:
		using Self = FileException;
		using Base = Exception;

		FileException(const std::string& msg) : Base(msg) { }

		FileException(std::string&& msg) : Base(std::move(msg)) { }

		FileException(const std::string& msg, const std::string& info) : Base(msg, info) { }

		FileException(std::string&& msg, const std::string& info): Base(std::move(msg), info) { }

		FileException(const Self&) = default;

		Self& operator=(const Self&) = default;

		FileException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};
}

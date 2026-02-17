/*********************************************************************
 * \file   SQLiteException.hpp
 * \brief  Contains SQLiteException class.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "../Exception.hpp"

namespace senc::utils::sqlite
{
	/**
	 * @class senc::utils::SQLiteException
	 * @brief Type of exception thrown on sqlite-related errors.
	 */
	class SQLiteException : public Exception
	{
	public:
		using Self = SQLiteException;
		using Base = Exception;

		SQLiteException(const std::string& msg) : Base(msg) { }

		SQLiteException(std::string&& msg) : Base(std::move(msg)) { }

		SQLiteException(const std::string& msg, const std::string& info) : Base(msg, info) { }

		SQLiteException(std::string&& msg, const std::string& info): Base(std::move(msg), info) { }

		SQLiteException(std::string&& msg, int code)
			: Base(std::move(msg), "Error code " + std::to_string(code)) { }

		SQLiteException(const Self&) = default;

		Self& operator=(const Self&) = default;

		SQLiteException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};
}

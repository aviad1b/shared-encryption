/*********************************************************************
 * \file   Exception.hpp
 * \brief  Header of base exception class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <exception>
#include <string>

namespace senc::utils
{
	/**
	 * @class senc::utils::Exception
	 * @brief Base class for exceptions.
	 */
	class Exception : public std::exception
	{
	public:
		using Self = Exception;

		Exception(const std::string& msg);

		Exception(std::string&& msg);

		Exception(const std::string& msg, const std::string& info);

		Exception(std::string&& msg, const std::string& info);

		Exception(const Self&) = default;

		Self& operator=(const Self&) = default;

		Exception(Self&&) = default;

		Self& operator=(Self&&) = default;

		const char* what() const noexcept override;

	private:
		std::string _msg;
	};
}

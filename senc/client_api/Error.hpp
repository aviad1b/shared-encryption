/*********************************************************************
 * \file   Error.hpp
 * \brief  Header of `Error` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "ValueOrError.hpp"
#include <string>

namespace senc::clientapi
{
	/**
	 * @class senc::clientapi::Error
	 * @brief Used to wrap error throws for API calls.
	 */
	class Error : public ValueOrError
	{
	public:
		using Self = Error;
		using Base = ValueOrError;

		/**
		 * @brief Constant (supposedly) which serves for returning allocation error.
		 */
		static Self* ALLOCATION;

		/**
		 * @brief Constructs an error instance.
		 * @param isAllocated Whether or not object was dynamically allocated.
		 * @param msg Error message (moved).
		 * @return Allocated instance, or fitting `Error` if failed.
		 */
		static Self make_instance(std::string&& msg) noexcept;

		/**
		 * @brief Constructs an allocated error instance.
		 * @param isAllocated Whether or not object was dynamically allocated.
		 * @param msg Error message (moved).
		 * @return Allocated instance, or fitting `Error` if failed.
		 */
		static ValueOrError* new_instance(std::string&& msg) noexcept;

		Error(const Self&) = delete;

		Self& operator=(const Self&) = delete;

		Error(Self&&) = delete;

		Self& operator=(Self&&) = delete;

		bool has_error() const override;

		/**
		 * @brief Gets error message as a c-string (similarly to exceptions).
		 * @return Error message c-string.
		 */
		const char* what() const;

	private:
		std::string _msg;

		static Self _ALLOCATION;

		/**
		 * @brief Constructs an error instance.
		 * @param isAllocated Whether or no object was dynamically allocated.
		 * @param msg Error message (moved).
		 */
		Error(bool isAllocated, std::string&& msg) noexcept;
	};
}

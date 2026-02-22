/*********************************************************************
 * \file   ValueOrError.hpp
 * \brief  Contains ValueOrError class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

namespace senc::clientapi
{
	/**
	 * @class senc::clientapi::ValueOrError
	 * @brief Used to wrap function returns and throws for API calls.
	 */
	class ValueOrError
	{
	public:
		using Self = ValueOrError;

		virtual ~ValueOrError() { }

		/**
		 * @brief Checks if has error (rather than value).
		 * @return `true` if has error, `false` if has value.
		 */
		virtual bool has_error() const = 0;

		/**
		 * @brief Checks if object was allocated dynamically.
		 * @return `true` if was allocated dynamically, otherwise `false`.
		 */
		bool allocated() const { return _isAllocated; }

	protected:
		ValueOrError(bool isAllocated) : _isAllocated(isAllocated) { }

	private:
		bool _isAllocated;
	};
}

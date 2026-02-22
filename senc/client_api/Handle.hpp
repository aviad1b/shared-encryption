/*********************************************************************
 * \file   Handle.hpp
 * \brief  Contains `Handle` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include <cstdint>

namespace senc::clientapi
{
	/**
	 * @class senc::clientapi::Handle
	 * @brief Used to wrap function returns and throws for API calls.
	 */
	class Handle
	{
	public:
		using Self = Handle;

		virtual ~Handle() { }

		/**
		 * @brief Checks if has error (rather than value).
		 * @return `true` if has error, `false` if has value.
		 */
		virtual bool has_error() const noexcept = 0;

		/**
		 * @brief Checks if object was allocated dynamically.
		 * @return `true` if was allocated dynamically, otherwise `false`.
		 */
		bool allocated() const { return _isAllocated; }

		/**
		 * @brief Gets "nint" (API) version of handle.
		 * @return API version of handle.
		 */
		std::uintptr_t as_nint() const noexcept
		{
			return reinterpret_cast<std::uintptr_t>(this);
		}

	protected:
		Handle(bool isAllocated) : _isAllocated(isAllocated) { }

	private:
		bool _isAllocated;
	};
}

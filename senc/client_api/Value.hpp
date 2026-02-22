/*********************************************************************
 * \file   Value.hpp
 * \brief  Header of `Value` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "Error.hpp"
#include <concepts>

namespace senc::clientapi
{
	/**
	 * @class senc::clientapi::Error
	 * @brief Used to wrap values for API calls.
	 */
	template <std::movable T>
	class Value : public Handle
	{
	public:
		using Self = Value<T>;
		using Base = Handle;

		/**
		 * @brief Constructs an allocated value instance.
		 * @param isAllocated Whether or not object was dynamically allocated.
		 * @param value Value (moved).
		 * @return Allocated instance, or fitting `Error` if failed.
		 */
		static Handle* new_instance(T&& value) noexcept;

		template <typename... Args>
		requires std::constructible_from<T, Args...>
		static Handle* new_instance(Args&&... args) noexcept;

		Value(const Self&) = delete;

		Self& operator=(const Self&) = delete;

		Value(Self&&) = delete;

		Self& operator=(Self&&) = delete;

		bool has_error() const override;

		/**
		 * @brief Gets stored value.
		 * @return Stored value (by ref).
		 */
		T& get() noexcept;

		/**
		 * @brief Gets stored value.
		 * @return Stored value (by ref).
		 */
		const T& get() const noexcept;

	private:
		T _value;

		/**
		 * @brief Constructs a value instance.
		 * @param isAllocated Whether or no object was dynamically allocated.
		 * @param value Value (moved).
		 */
		Value(bool isAllocated, T&& value) noexcept(std::is_nothrow_move_constructible_v<T>);
	};
}

#include "Value_impl.hpp"

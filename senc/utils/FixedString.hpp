/*********************************************************************
 * \file   FixedString.hpp
 * \brief  Contains FixedString struct (constexpr string for templates).
 *
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include <string_view>
#include <algorithm>

namespace senc::utils
{
	/**
	 * @struct senc::utils::FixedString
	 * @brief Constexpr string struct which can be used as template arg.
	 * @tparam n String size including null-termination.
	 */
	template <std::size_t n>
	struct FixedString
	{
		/**
		 * @brief String's length (without null-termination).
		 */
		static constexpr std::size_t LEN = n - 1; // discluding null-termination

		char value[n];

		/**
		 * @brief Constructs a fixed string.
		 * @param str A reference to characters array (with size n including null-termination).
		 */
		constexpr FixedString(const char(&str)[n])
		{
			std::copy_n(str, n, value);
		}

		/**
		 * @brief Gets string's length (without null-termination).
		 * @return String's length (without null-termination).
		 */
		constexpr std::size_t len() const
		{
			return LEN;
		}

		/**
		 * @brief Checks if string is empty.
		 * @return `true` if empty, otherwise `false`.
		 */
		constexpr bool empty() const
		{
			return 0 == LEN;
		}

		/**
		 * @brief Compares the fixed string with another one (of any length).
		 */
		constexpr auto operator<=>(const FixedString&) const = default;

		/**
		 * @brief Gets a string view version of the fixed string.
		 * @return A string view version of the fixed string.
		 */
		constexpr std::string_view view() const
		{
			return { value, LEN };
		}

		/**
		 * @brief Gets a string view version of the fixed string.
		 * @return A string view version of the fixed string.
		 */
		constexpr operator std::string_view() const
		{
			return view();
		}

		/**
		 * @brief Gets a C-string version of the fixed string.
		 * @return A C-string version of the fixed string.
		 */
		constexpr const char* c_str() const
		{
			return value;
		}

		/**
		 * @brief Accesses a character at a given index.
		 * @param i Character index, expected to be within bounds.
		 * @return Character at index `i` in the fixed string.
		 */
		constexpr char operator[](std::size_t i) const
		{
			return value[i];
		}
	};

	/**
	 * @struct senc::utils::FixedStringConstant
	 * @brief An STD-like FixedString value wrapper.
	 * @tparam v Fixed string value.
	 */
	template <FixedString v>
	struct FixedStringConstant
	{
		static constexpr FixedString& value = v;
	};
}

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
#include <concepts>

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

	namespace sfinae
	{
		template <typename T>
		struct is_fixed_string : std::false_type { };

		template <std::size_t n>
		struct is_fixed_string<FixedString<n>> : std::true_type { };
	}

	/**
	 * @concept senc::utils::AnyFixedString
	 * @brief Looks for any instantation of `utils::FixedString`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept AnyFixedString = sfinae::is_fixed_string<Self>::value;

	/**
	 * @struct senc::utils::FixedStringConstant
	 * @brief An STD-like FixedString value wrapper.
	 * @tparam v Fixed string value.
	 */
	template <FixedString v = "">
	struct FixedStringConstant
	{
		static constexpr FixedString value = v;
	};

	/**
	 * @struct senc::utils::EmptyFixedStringConstant
	 * @brief Empty variation of FixedStringConstant.
	 */
	struct EmptyFixedStringConstant : FixedStringConstant<> { };

	template <AnyFixedString... Strs>
	struct FixedStringList
	{
		/**
		 * @brief List's length (amount of strings in list).
		 */
		static constexpr std::size_t LEN = sizeof...(Strs);

		/**
		 * @brief Gets list's length.
		 * @return Amount of strings in list.
		 */
		constexpr std::size_t size() const { return LEN; }

		/**
		 * @brief Checks if contains a given string.
		 * @tparam Str String to look for in list.
		 * @return `true` if `Str` is in the list, otherwise `false`.
		 */
		template <AnyFixedString Str>
		consteval bool contains() const
		{
			return ((Strs == Str) || ...);
		}
	};

	namespace sfinae
	{
		template <typename T>
		struct is_fixed_string_list : std::false_type { };

		template <AnyFixedString... Strs>
		struct is_fixed_string_list<FixedStringList<Strs...>> : std::true_type { };
	}

	/**
	 * @concept senc::utils::AnyFixedStringList
	 * @brief Looks for any instantation of `FixedStringList`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept AnyFixedStringList = sfinae::is_fixed_string_list<Self>::value;

	namespace sfinae
	{
		template <AnyFixedStringList List, AnyFixedString Str>
		struct fixed_string_list_contains : std::false_type { };

		template <AnyFixedString Str, AnyFixedString... Strs>
		struct fixed_string_list_contains<FixedStringList<Strs...>, Str>
			: std::bool_constant<
				((Strs == Str) || ...)
			> { };
	}

	/**
	 * @concept senc::utils::FixedStringListWith
	 * @brief Looks for a fixed string list which contains a specific fixed string.
	 * @tparam Self Examined typename.
	 * @tparam Str String to look for in list.
	 */
	template <typename Self, typename Str>
	concept FixedStringListWith = AnyFixedStringList<Self> &&
		AnyFixedString<Str> &&
		sfinae::fixed_string_list_contains<Self, Str>::value;
}

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
#include <tuple>

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
		 * @brief Order-compares the fixed string with another one (of any length).
		 */
		constexpr auto operator<=>(const FixedString&) const = default;

		/**
		 * @brief Equality-compares the fixed string with another one (of any length).
		 */
		constexpr bool operator==(const FixedString&) const = default;

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

		/**
		 * @brief Concatinates this fixed string with another.
		 * @param other Other fixed string to concatinate with this one.
		 * @return Concatinated fixed string.
		 */
		template <std::size_t m>
		constexpr auto operator+(const FixedString<m>& other) const
		{
			FixedString<LEN + m> res{};
			std::copy_n(value, LEN, res.value);
			std::copy_n(other.value, m, res.value);
			return res;
		}

		/**
		 * @brief Concatinates this fixed string with a string literal.
		 * @param str String literal to concatinate with this fixed string.
		 * @return Concatinated fixed string.
		 */
		template <std::size_t m>
		constexpr auto operator+(const char(&str)[m]) const
		{
			return *this + FixedString<m>(str);
		}
	};

	/**
	 * @brief Concatinates a string literal with a fixed string.
	 * @param str String literal to concatinate with a fixed string.
	 * @param fs Fixed string to concatinate with `str`.
	 * @return Concatinated fixed string.
	 */
	template <std::size_t n, std::size_t m>
	constexpr auto operator+(const char(&str)[n], const FixedString<m>& fs)
	{
		return FixedString<n>(str) + fs;
	}

	namespace sfinae
	{
		template <typename T>
		struct is_fixed_string_type : std::false_type { };

		template <std::size_t n>
		struct is_fixed_string_type<FixedString<n>> : std::true_type { };

		template <std::size_t n>
		struct is_fixed_string_type<FixedString<n>&> : std::true_type { };

		template <std::size_t n>
		struct is_fixed_string_type<const FixedString<n>&> : std::true_type { };
	}

	/**
	 * @concept senc::utils::FixedStringType
	 * @brief Looks for any instantation of `utils::FixedString`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept FixedStringType = sfinae::is_fixed_string_type<Self>::value;

	template <typename Self>
	concept EmptyFixedStringType = FixedStringType<Self> && (0 == Self::LEN);

	template <typename Self>
	concept NonEmptyFixedStringType = FixedStringType<Self> && !EmptyFixedStringType<Self>;

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
}

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
		 * @brief Constructs an empty fixed string.
		 */
		constexpr FixedString() requires (n > 0)
		{
			for (std::size_t i = 0; i < n; ++i)
				value[i] = '\0';
		}

		/**
		 * @brief Constructs a fixed string.
		 * @param str A reference to characters array (with size n including null-termination).
		 */
		constexpr FixedString(const char(&str)[n])
		{
			for (std::size_t i = 0; i < n; ++i)
				value[i] = str[i];
		}

		/**
		 * @brief Copy constructor of fixed string.
		 * @param other Other fixed string to copy.
		 */
		constexpr FixedString(const FixedString<n>& other)
		{
			for (std::size_t i = 0; i < n; ++i)
				value[i] = other.value[i];
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
			if constexpr (0 == n)
				return true;
			else return !value[0];
		}

		/**
		 * @brief Order-compares the fixed string with another one (of any length).
		 */
		template <std::size_t m>
		constexpr auto operator<=>(const FixedString<m>& other) const
		{
			return view() <=> other.view();
		}

		/**
		 * @brief Equality-compares the fixed string with another one (of any length).
		 */
		template <std::size_t m>
		constexpr bool operator==(const FixedString<m>& other) const
		{
			return view() == other.view();
		}

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
			std::size_t i = 0, j = 0;
			for (i = 0; value[i]; ++i)
				res.value[i] = value[i];
			for (j = 0; other.value[j]; ++j)
				res.value[i + j] = other.value[j];
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

	/**
	 * @concept senc::utils::EmptyFixedStringType
	 * @brief Looks for an empty fixed string type (n=1, enough space for null-termination only).
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept EmptyFixedStringType = FixedStringType<Self> && (0 == Self::LEN);

	/**
	 * @concept senc::utils::NonEmptyFixedStringType
	 * @brief Looks for a fixed string type that isn't `EmptyFixedStringType`.
	 * @see `EmptyFixedStringType`.
	 */
	template <typename Self>
	concept NonEmptyFixedStringType = FixedStringType<Self> && !EmptyFixedStringType<Self>;

	namespace sfinae
	{
		// used for checking if two fixed strings are the same (implicitly calls operator==)
		template <FixedString fs1, FixedString fs2>
		struct is_same_fixed_string : std::false_type { };

		template <FixedString fs>
		struct is_same_fixed_string<fs, fs> : std::true_type { };
	}

	/**
	 * @struct senc::utils::FixedStringConstant
	 * @brief An STD-like FixedString value wrapper.
	 * @tparam v Fixed string value (defaults to empty).
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

	namespace sfinae
	{
		// used for conditional evaluation to a fixed string value
		template <bool flag, FixedString fs>
		struct cond_fixed_string : EmptyFixedStringConstant { };

		template <FixedString fs>
		struct cond_fixed_string<true, fs> : FixedStringConstant<fs> { };
	}

	/**
	 * @var senc::utils::COND_FIXED_STRING
	 * @brief Conditionally evaluates to a fixed string value.
	 * @tparam flag Flag for conditional evaluation, true will retuls given string, false will result empty.
	 * @tparam fs Fixed string value to evaluate to if `flag` is `true`.
	 */
	template <bool flag, FixedString fs>
	constexpr FixedString COND_FIXED_STRING = sfinae::cond_fixed_string<flag, fs>::value;

	namespace sfinae
	{
		// used for concatinating the same fixed string multiple times
		template <FixedString fs, std::size_t t, FixedString sep = "">
		struct fixed_string_dup : FixedStringConstant<
			fs + sep + fixed_string_dup<fs, t - 1, sep>::value
		> { };

		template <FixedString fs, FixedString sep>
		struct fixed_string_dup<fs, 0, sep> : EmptyFixedStringConstant { };

		template <FixedString fs, FixedString sep>
		struct fixed_string_dup<fs, 1, sep> : FixedStringConstant<fs> { };
	}

	/**
	 * @var senc::utils::FIXED_STRING_DUP
	 * @brief Concatinates the same fixed string multiple times.
	 * @tparam fs Fixed string to duplicate.
	 * @tparam t Amount of times to concatinate.
	 * @tparam sep Optional seperator to have between repetitions of `fs`.
	 */
	template <FixedString fs, std::size_t t, FixedString sep = "">
	constexpr FixedString FIXED_STRING_DUP = sfinae::fixed_string_dup<fs, t, sep>::value;

	namespace sfinae
	{
		// used for joining multiple fixed strings together, separated by a given separator
		template <FixedString sep, FixedString... tokens>
		struct fixed_string_join : EmptyFixedStringConstant { };

		template <FixedString sep, FixedString token>
		struct fixed_string_join<sep, token> : FixedStringConstant<token> { };

		template <FixedString sep, FixedString first, FixedString... rest>
		struct fixed_string_join<sep, first, rest...> : FixedStringConstant<
			first + sep + fixed_string_join<sep, rest...>::value
		> { };

		// used for joining multiple non-empty fixed strings together, separated by a given separator
		template <FixedString sep, FixedString... tokens>
		struct fixed_string_non_empty_join : EmptyFixedStringConstant { };

		template <FixedString sep, FixedString token>
		struct fixed_string_non_empty_join<sep, token> : FixedStringConstant<token> { };

		template <FixedString sep, FixedString first, FixedString... rest>
		struct fixed_string_non_empty_join<sep, first, rest...> : FixedStringConstant<
			first + COND_FIXED_STRING<!first.empty(), sep> + fixed_string_non_empty_join<sep, rest...>::value
		> { };
	}

	/**
	 * @var senc::utils::FIXED_STRING_JOIN
	 * @brief Joins multiple fixed strings together, separated by a given separator.
	 * @tparam sep Separator to have between tokens.
	 * @tparam tokens Fixed string tokens to join.
	 */
	template <FixedString sep, FixedString... tokens>
	constexpr FixedString FIXED_STRING_JOIN =
		sfinae::fixed_string_join<sep, tokens...>::value;

	/**
	 * @var senc::utils::FIXED_STRING_NON_EMPTY_JOIN
	 * @brief Joins multiple non-empty fixed strings together, separated by a given separator.
	 * @tparam sep Separator to have between tokens.
	 * @tparam tokens Fixed string tokens to join.
	 */
	template <FixedString sep, FixedString... tokens>
	constexpr FixedString FIXED_STRING_NON_EMPTY_JOIN =
		sfinae::fixed_string_non_empty_join<sep, tokens...>::value;
}

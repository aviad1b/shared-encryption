/*********************************************************************
 * \file   concepts.hpp
 * \brief  Contains general concept declarations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <concepts>
#include <iterator>
#include <ostream>
#include <istream>
#include <string>
#include <ranges>
#include <tuple>

namespace senc::utils
{
	/**
	 * @concept senc::utils::OneOf
	 * @brief Checks for a typename from a provided typename list.
	 * @tparam Self Examined typename.
	 * @tparam Ts Types that `Self` is expected to be one of.
	 */
	template <typename Self, typename... Ts>
	concept OneOf = (std::same_as<Self, Ts> || ...);

	/**
	 * @concept senc::utils::TupleLike
	 * @brief Checks for a tuple-like typename.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept TupleLike = requires { typename std::tuple_size<Self>::type; } &&
		(std::tuple_size_v<Self> >= 0);

	/**
	 * @concept senc::utils::InputIterator
	 * @brief Looks for a typename that is an input iterator for a given element type.
	 * @tparam Self Examined typename.
	 * @tparam T Element type.
	 */
	template <typename Self, typename T>
	concept InputIterator = std::input_iterator<Self> &&
		std::same_as<std::iter_value_t<Self>, T>;

	/**
	 * @concept senc::utils::InputRange
	 * @brief Looks for a typename that is an input range of a given element type.
	 * @tparam Self Examined typename.
	 * @tparam T Element type.
	 */
	template <typename Self, typename T>
	concept InputRange =
		std::ranges::input_range<Self> &&
		InputIterator<std::ranges::iterator_t<Self>, T>;

	namespace sfinae
	{
		template <typename T>
		struct is_string_type : std::false_type { };

		template <typename Char, typename Traits, typename Alloc>
		struct is_string_type<std::basic_string<Char, Traits, Alloc>> : std::true_type { };
	}

	/**
	 * @concept senc::utils::StringType
	 * @brief Looks for a basic string type (from `std`).
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept StringType = sfinae::is_string_type<Self>::value;

	/**
	 * @concept senc::utils::ConvertibleTo
	 * @brief Looks for a typename that can be casted to another.
	 * @tparam Self Examined typename.
	 * @tparam To Destination typename.
	 */
	template <typename Self, typename To>
	concept ConvertibleTo = requires(Self&& self)
	{
		{ static_cast<To>(std::forward<Self>(self)) };
	};

	/**
	 * @concept senc::utils::ConvertibleToNoExcept
	 * @brief Looks for a typename that can be casted to another, without throwing.
	 * @tparam Self Examined typename.
	 * @tparam To Destination typename.
	 */
	template <typename Self, typename To>
	concept ConvertibleToNoExcept = requires(Self && self)
	{
		{ static_cast<To>(std::forward<Self>(self)) } noexcept;
	};

	/**
	 * @concept senc::utils::BoolConvertible
	 * @brief Looks for a typename that can be casted to `bool`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept BoolConvertible = ConvertibleTo<Self, bool>;

	/**
	 * @concept senc::utils::BoolConvertibleNoExcept
	 * @brief Looks for a typename that can be casted to `bool`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept BoolConvertibleNoExcept = ConvertibleToNoExcept<Self, bool>;

	/**
	 * @concept senc::utils::RetConvertible
	 * @brief Looks for a typename which is equivalent to another as a return type.
	 * @tparam Self Examined typename.
	 * @tparam T Equivalent-in-question type.
	 * @note `void` is ret-convertible to any typename, since calling any function is 
	 *       equivalent to calling a void function.
	 */
	template <typename Self, typename T>
	concept RetConvertible = std::same_as<T, void> || ConvertibleTo<Self, T>;

	/**
	 * @concept senc::utils::RetConvertibleNoExcept
	 * @brief Looks for a typename which is equivalent to another as a return type, without throwing.
	 * @tparam Self Examined typename.
	 * @tparam T Equivalent-in-question type.
	 * @note `void` is ret-convertible to any typename, since calling any function is
	 *       equivalent to calling a void function.
	 */
	template <typename Self, typename T>
	concept RetConvertibleNoExcept = std::same_as<T, void> || ConvertibleToNoExcept<Self, T>;

	/**
	 * @concept senc::utils::Callable
	 * @brief Checks for a typename which is callable with given arg types for given return type.
	 * @tparam Self Examined typename.
	 * @tparam Ret Expected return type.
	 * @tparam Args Expected argument types.
	 */
	template <typename Self, typename Ret, typename... Args>
	concept Callable = requires
	{
		{ std::declval<Self>()(std::declval<Args>()...) } -> RetConvertible<Ret>;
	};

	/**
	 * @concept senc::utils::CallableNoExcept
	 * @brief Looks for a typename which is callable with given arg types for given return type,
	 *		  without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Ret Expected return type.
	 * @tparam Args Expected argument types.
	 */
	template <typename Self, typename Ret, typename... Args>
	concept CallableNoExcept = requires
	{
		{ std::declval<Self>()(std::declval<Args>()...) } noexcept -> RetConvertibleNoExcept<Ret>;
	};

	/**
	 * @concept senc::utils::ConstCallable
	 * @brief Looks for a typename which is const-callable with given arg types for given return type.
	 * @tparam Self Examined typename.
	 * @tparam Ret Expected return type.
	 * @tparam Args Expected argument types.
	 */
	template <typename Self, typename Ret, typename... Args>
	concept ConstCallable = requires(const Self self)
	{
		{ self(std::declval<Args>()...) } -> RetConvertible<Ret>;
	};

	/**
	 * @concept senc::utils::ConstCallableNoExcept
	 * @brief Checks for a typename which is const-callable with given arg types for given return type,
	 *		  without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Ret Expected return type.
	 * @tparam Args Expected argument types.
	 */
	template <typename Self, typename Ret, typename... Args>
	concept ConstCallableNoExcept = requires(const Self self)
	{
		{ self(std::declval<Args>()...) } noexcept -> RetConvertibleNoExcept<Ret>;
	};

	/**
	 * @concept senc::utils::HasContainsMethod
	 * @brief Checks for a typename with a method that checks if contains a value.
	 * @tparam Self Examined typename.
	 * @tparam T Value type.
	 */
	template <typename Self, typename T>
	concept HasContainsMethod = requires(const Self self, const T value)
	{
		{ self.contains(value) } -> BoolConvertible;
	};

	/**
	 * @concept senc::utils::HasContainsMethodNoExcept
	 * @brief Checks for a typename with a method that checks if contains a value, without throwing.
	 * @tparam Self Examined typename.
	 * @tparam T Value type.
	 */
	template <typename Self, typename T>
	concept HasContainsMethodNoExcept = requires(const Self self, const T value)
	{
		{ self.contains(value) } noexcept -> BoolConvertibleNoExcept;
	};

	/**
	 * @concept senc::utils::Equaler
	 * @brief Checks for a typename which can be used to equality-check two instances of a given type.
	 * @tparam Self Examined typename.
	 * @tparam Value Typename to compare instances of using `Self`.
	 * @tparam Other Typename to equality-check `Value` to, defaults to `Value`.
	 */
	template <typename Self, typename Value, typename Other = Value>
	concept Equaler = ConstCallable<Self, bool, const Value&, const Other&>;

	/**
	 * @concept senc::utils::Copyable
	 * @brief Looks for a typename that can be copied (and moved).
	 * @note Equivalent to `std::copy`, but without requiring to be swappable.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Copyable = requires(Self a, const Self b)
	{
		{ Self(b) };
		{ Self(std::move(a)) };
		{ a = b };
		{ a = std::move(b) };
	};

	/**
	 * @concept senc::utils::CopyableNoExcept
	 * @brief Looks for a typename that can be copied (and moved), without throwing.
	 * @note Equivalent to `std::copy`, but without requiring to be swappable (and without throwing).
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept CopyableNoExcept = requires(Self a, const Self b)
	{
		{ Self(b) } noexcept;
		{ Self(std::move(a)) } noexcept;
		{ a = b } noexcept;
		{ a = std::move(b) } noexcept;
	};

	/**
	 * @concept senc::utils::Outputable
	 * @brief Looks for a typename that can be outputted into an ostream.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Outputable = requires(std::ostream& os, const Self self)
	{
		{ os << self } -> std::convertible_to<std::ostream&>;
	};

	/**
	 * @concept senc::utils::Inputable
	 * @brief Looks for a typename that can be inputted from an istream.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Inputable = requires(std::istream & is, Self self)
	{
		{ is >> self } -> std::convertible_to<std::istream&>;
	};

	/**
	 * @concept senc::utils::DefaultConstructibleClass
	 * @brief Looks for a non-fundamental class typename which can be default-constructed.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept DefaultConstructibleClass = std::is_default_constructible_v<Self> &&
		!std::is_fundamental_v<Self>;

	/**
	 * @concept senc::utils::DefaultConstructibleClassNoExcept
	 * @brief Looks for a non-fundamental class typename which can be default-constructed, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept DefaultConstructibleClassNoExcept = std::is_nothrow_default_constructible_v<Self> &&
		!std::is_fundamental_v<Self>;

	/**
	 * @concept senc::utils::ZeroConstructible
	 * @brief Looks for a typename that can be constructed form zero.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ZeroConstructible = requires { { Self(0) }; };

	/**
	 * @concept senc::utils::ZeroConstructibleNoExcept
	 * @brief Looks for a typename that can be constructed form zero, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ZeroConstructibleNoExcept = requires { { Self(0) } noexcept; };

	/**
	 * @concept senc::utils::ClassDefaultOrZeroConstructible
	 * @brief Looks for a typename that is either `DefaultConstructibleClass` or `ZeroConstructible`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ClassDefaultOrZeroConstructible = DefaultConstructibleClass<Self> ||
		ZeroConstructible<Self>;

	/**
	 * @concept senc::utils::ClassDefaultOrZeroConstructibleNoExcept
	 * @brief Looks for a typename that is either `DefaultConstructibleClassNoExcept` or 
	 *        `ZeroConstructibleNoExcept`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ClassDefaultOrZeroConstructibleNoExcept =
		(DefaultConstructibleClass<Self> && DefaultConstructibleClassNoExcept<Self>) ||
		(ZeroConstructible<Self> && ZeroConstructibleNoExcept<Self>);

	/**
	 * @concept senc::utils::OneConstructible
	 * @brief Looks for a typename that can be initialized form one.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept OneConstructible = requires { { Self(1) }; };

	/**
	 * @concept senc::utils::OneConstructibleNoExcept
	 * @brief Looks for a typename that can be initialized form one, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept OneConstructibleNoExcept = requires { { Self(1) } noexcept; };

	/**
	 * @concept senc::utils::HasIdentity
	 * @brief Looks for a typename that has a static `identity` method.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasIdentity = requires { { Self::identity() } -> ConvertibleTo<const Self&>; };

	/**
	 * @concept senc::utils::HasIdentityNoExcept
	 * @brief Looks for a typename that has a static `identity` method which does not throw.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasIdentityNoExcept = requires { { Self::identity() } noexcept -> ConvertibleToNoExcept<const Self&>; };

	/**
	 * @concept senc::utils::IntConstructible
	 * @brief Looks for a typename that can be constructed from `int`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept IntConstructible = std::constructible_from<Self, int>;

	/**
	 * @concept senc::utils::IntConstructibleNoExcept
	 * @brief Looks for a typename that can be constructed from `int`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept IntConstructibleNoExcept = std::is_nothrow_constructible_v<Self, int>;

	/**
	 * @concept senc::utils::ZeroComparable
	 * @brief Looks for a typename that can be compared to zero.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ZeroComparable = requires(const Self self)
	{
		{ 0 == self } -> BoolConvertible;
	};

	/**
	 * @concept senc::utils::ZeroComparableNoExcept
	 * @brief Looks for a typename that can be compared to zero, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ZeroComparableNoExcept = requires(const Self self)
	{
		{ 0 == self } noexcept -> BoolConvertibleNoExcept;
	};

	/**
	 * @concept senc::utils::EqualityComparable
	 * @brief Looks for a typename that can be compared to another (or itself) using `operator==`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator==`.
	 */
	template <typename Self, typename Other = Self>
	concept EqualityComparable = requires(const Self a, const Other b)
	{
		{ a == b } -> BoolConvertible;
	};

	/**
	 * @concept senc::utils::EqualityComparableNoExcept
	 * @brief Looks for a typename that can be compared to another (or itself) using `operator==`, 
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator==`.
	 */
	template <typename Self, typename Other = Self>
	concept EqualityComparableNoExcept = requires(const Self a, const Other b)
	{
		{ a == b } -> BoolConvertibleNoExcept;
	};

	/**
	 * @concept senc::utils::LowerComparable
	 * @brief Looks for a typename that can be compared to another (or itself) using `operator<`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator<`.
	 */
	template <typename Self, typename Other = Self>
	concept LowerComparable = requires(const Self a, const Other b)
	{
		{ a < b } -> BoolConvertible;
	};

	/**
	 * @concept senc::utils::LowerComparableNoExcept
	 * @brief Looks for a typename that can be compared to another (or itself) using `operator<`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator<`.
	 */
	template <typename Self, typename Other = Self>
	concept LowerComparableNoExcept = requires(const Self a, const Other b)
	{
		{ a < b } noexcept -> BoolConvertibleNoExcept;
	};

	/**
	 * @concept senc::utils::LeftIncrementable
	 * @brief Looks for a typename that can be pre-incemented using `operator++`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept LeftIncrementable = requires(Self a)
	{
		{ ++a } -> ConvertibleTo<Self&>;
	};

	/**
	 * @concept senc::utils::LeftIncrementableNoExcept
	 * @brief Looks for a typename that can be pre-incemented using `operator++`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept LeftIncrementableNoExcept = requires(Self a)
	{
		{ ++a } noexcept -> ConvertibleToNoExcept<Self&>;
	};

	/**
	 * @concept senc::utils::RightIncrementable
	 * @brief Looks for a typename that can be post-incemented using `operator++`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept RightIncrementable = requires(Self a)
	{
		{ a++ } -> ConvertibleTo<Self>;
	};

	/**
	 * @concept senc::utils::RightIncrementableNoExcept
	 * @brief Looks for a typename that can be post-incemented using `operator++`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept RightIncrementableNoExcept = requires(Self a)
	{
		{ a++ } noexcept -> ConvertibleToNoExcept<Self>;
	};

	/**
	 * @concept senc::utils::LeftDecrementable
	 * @brief Looks for a typename that can be pre-decemented using `operator--`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept LeftDecrementable = requires(Self a)
	{
		{ --a } -> ConvertibleTo<Self&>;
	};

	/**
	 * @concept senc::utils::LeftDecrementableNoExcept
	 * @brief Looks for a typename that can be pre-decemented using `operator--`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept LeftDecrementableNoExcept = requires(Self a)
	{
		{ --a } noexcept -> ConvertibleToNoExcept<Self&>;
	};

	/**
	 * @concept senc::utils::RightDecrementable
	 * @brief Looks for a typename that can be post-decemented using `operator--`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept RightDecrementable = requires(Self a)
	{
		{ a-- } -> ConvertibleTo<Self>;
	};

	/**
	 * @concept senc::utils::RightDecrementableNoExcept
	 * @brief Looks for a typename that can be post-decemented using `operator--`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept RightDecrementableNoExcept = requires(Self a)
	{
		{ a-- } noexcept -> ConvertibleToNoExcept<Self>;
	};

	/**
	 * @concept senc::utils::Negatable
	 * @brief Looks for a typename that can be negated using `operator-`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Negatable = requires(const Self a)
	{
		{ -a } -> ConvertibleTo<Self>;
	};

	/**
	 * @concept senc::utils::NegatableNoExcept
	 * @brief Looks for a typename that can be negated using `operator-`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept NegatableNoExcept = requires(const Self a)
	{
		{ -a } noexcept -> ConvertibleToNoExcept<Self>;
	};

	/**
	 * @concept senc::utils::Addable
	 * @brief Looks for a typename that can be added to another (or itself) using `operator+`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator+`.
	 */
	template <typename Self, typename Other = Self>
	concept Addable = requires(const Self a, const Other b)
	{
		{ a + b } -> ConvertibleTo<Self>;
	};

	/**
	 * @concept senc::utils::Addable
	 * @brief Looks for a typename that can be added to another (or itself) using `operator+`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator+`.
	 */
	template <typename Self, typename Other = Self>
	concept AddableNoExcept = requires(const Self a, const Other b)
	{
		{ a + b } noexcept -> ConvertibleToNoExcept<Self>;
	};

	/**
	 * @concept senc::utils::SelfAddable
	 * @brief Looks for a typename that can be added with another (or itself) using `operator+=`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator+=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfAddable = requires(Self a, const Other b)
	{
		{ a += b } -> ConvertibleTo<Self&>;
	};

	/**
	 * @concept senc::utils::SelfAddableNoExcept
	 * @brief Looks for a typename that can be added with another (or itself) using `operator+=`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator+=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfAddableNoExcept = requires(Self a, const Other b)
	{
		{ a += b } noexcept -> ConvertibleToNoExcept<Self&>;
	};

	/**
	 * @concept senc::utils::Subtractable
	 * @brief Looks for a typename that can be subtracted from another (or itself) using `operator-`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator-`.
	 */
	template <typename Self, typename Other = Self>
	concept Subtractable = requires(const Self a, const Other b)
	{
		{ a - b } -> ConvertibleTo<Self>;
	};

	/**
	 * @concept senc::utils::SubtractableNoExcept
	 * @brief Looks for a typename that can be subtracted from another (or itself) using `operator-`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator-`.
	 */
	template <typename Self, typename Other = Self>
	concept SubtractableNoExcept = requires(const Self a, const Other b)
	{
		{ a - b } noexcept -> ConvertibleToNoExcept<Self>;
	};

	/**
	 * @concept senc::utils::SelfSubtractable
	 * @brief Looks for a typename that can be subtracted by another (or itself) using `operator-=`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator-=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfSubtractable = requires(Self a, const Other b)
	{
		{ a -= b } -> ConvertibleTo<Self&>;
	};

	/**
	 * @concept senc::utils::SelfSubtractableNoExcept
	 * @brief Looks for a typename that can be subtracted by another (or itself) using `operator-=`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator-=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfSubtractableNoExcept = requires(Self a, const Other b)
	{
		{ a -= b } noexcept -> ConvertibleToNoExcept<Self&>;
	};

	/**
	 * @concept senc::utils::Multiplicable
	 * @brief Looks for a typename that can be multiplied by another (or itself) using `operator*`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator*`.
	 */
	template <typename Self, typename Other = Self>
	concept Multiplicable = requires(const Self a, const Other b)
	{
		{ a * b } -> ConvertibleTo<Self>;
	};

	/**
	 * @concept senc::utils::MultiplicableNoExcept
	 * @brief Looks for a typename that can be multiplied by another (or itself) using `operator*`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator*`.
	 */
	template <typename Self, typename Other = Self>
	concept MultiplicableNoExcept = requires(const Self a, const Other b)
	{
		{ a * b } noexcept -> ConvertibleToNoExcept<Self>;
	};

	/**
	 * @concept senc::utils::SelfMultiplicable
	 * @brief Looks for a typename that can be multiplied by another (or itself) using `operator*=`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator*=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfMultiplicable = requires(Self a, const Other b)
	{
		{ a *= b } -> ConvertibleTo<Self&>;
	};

	/**
	 * @concept senc::utils::SelfMultiplicableNoExcept
	 * @brief Looks for a typename that can be multiplied by another (or itself) using `operator*=`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator*=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfMultiplicableNoExcept = requires(Self a, const Other b)
	{
		{ a *= b } noexcept -> ConvertibleToNoExcept<Self&>;
	};

	/**
	 * @concept senc::utils::Devisible
	 * @brief Looks for a typename that can be devided by another (or itself) using `operator/`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator/`.
	 */
	template <typename Self, typename Other = Self>
	concept Devisible = requires(const Self a, const Other b)
	{
		{ a / b } -> ConvertibleTo<Self>;
	};

	/**
	 * @concept senc::utils::DevisibleNoExcept
	 * @brief Looks for a typename that can be devided by another (or itself) using `operator/`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator/`.
	 */
	template <typename Self, typename Other = Self>
	concept DevisibleNoExcept = requires(const Self a, const Other b)
	{
		{ a / b } noexcept -> ConvertibleToNoExcept<Self>;
	};

	/**
	 * @concept senc::utils::SelfDevisible
	 * @brief Looks for a typename that can be devided by another (or itself) using `operator/=`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator/=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfDevisible = requires(Self a, const Other b)
	{
		{ a /= b } -> ConvertibleTo<Self&>;
	};

	/**
	 * @concept senc::utils::SelfDevisibleNoExcept
	 * @brief Looks for a typename that can be devided by another (or itself) using `operator/=`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator/=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfDevisibleNoExcept = requires(Self a, const Other b)
	{
		{ a /= b } noexcept -> ConvertibleToNoExcept<Self&>;
	};

	/**
	 * @concept senc::utils::Modulable
	 * @brief Looks for a typename that can be modulo-devided by another (or itself) using `operator%`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator%`.
	 */
	template <typename Self, typename Other = Self>
	concept Modulable = requires(const Self a, const Other b)
	{
		{ a % b } -> ConvertibleTo<Self>;
	};

	/**
	 * @concept senc::utils::ModulableNoExcept
	 * @brief Looks for a typename that can be modulo-devided by another (or itself) using `operator%`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied with `Self` using `operator%`.
	 */
	template <typename Self, typename Other = Self>
	concept ModulableNoExcept = requires(const Self a, const Other b)
	{
		{ a % b } noexcept -> ConvertibleToNoExcept<Self>;
	};

	/**
	 * @concept senc::utils::SelfModulable
	 * @brief Looks for a typename that can be modulo-devided by another (or itself) using `operator%=`.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator%=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfModulable = requires(Self a, const Other b)
	{
		{ a %= b } -> ConvertibleTo<Self&>;
	};

	/**
	 * @concept senc::utils::SelfModulableNoExcept
	 * @brief Looks for a typename that can be modulo-devided by another (or itself) using `operator%=`,
	 *        without throwing.
	 * @tparam Self Examined typename.
	 * @tparam Other Typename to check if can be applied on `Self` using `operator%=`.
	 */
	template <typename Self, typename Other = Self>
	concept SelfModulableNoExcept = requires(Self a, const Other b)
	{
		{ a %= b } noexcept -> ConvertibleToNoExcept<Self&>;
	};
}

// extracts concept from a concept-types pack
#define SENC_CONCEPT_NAME(pack) SENC_CONCEPT_NAME_IMPL pack
#define SENC_CONCEPT_NAME_IMPL(con, ...) con

// extracts types from a concept-types pack
#define SENC_CONCEPT_TYPES(pack) SENC_CONCEPT_TYPES_IMPL pack
#define SENC_CONCEPT_TYPES_IMPL(con, ...) __VA_ARGS__

// creates noexcept check for concept (appends `NoExcept` to concept name)
#define SENC_NOEXCEPT_CHECK(pack) SENC_NOEXCEPT_CHECK_EXPAND(SENC_CONCEPT_NAME(pack), SENC_CONCEPT_TYPES(pack))
#define SENC_NOEXCEPT_CHECK_EXPAND(con, ...) SENC_NOEXCEPT_CHECK_IMPL(con, __VA_ARGS__)
#define SENC_NOEXCEPT_CHECK_IMPL(con, ...) con##NoExcept<__VA_ARGS__>

// creates requires check for concept
#define SENC_REQUIRES_CHECK(pack) SENC_REQUIRES_CHECK_EXPAND(SENC_CONCEPT_NAME(pack), SENC_CONCEPT_TYPES(pack))
#define SENC_REQUIRES_CHECK_EXPAND(con, ...) SENC_REQUIRES_CHECK_IMPL(con, __VA_ARGS__)
#define SENC_REQUIRES_CHECK_IMPL(con, ...) con<__VA_ARGS__>


// SENC_REQ generates requires checks for provided concepts
// SENC_REQ_NOEXCEPT generates both noexcept and requires checks for provided concepts
// SENC_REQ_NOEXCEPT_COND generates both noexcept and requires checks for provided concepts,
//                        with an explicit additional requires condition

#define SENC_REQ_NOEXCEPT_COND_1(cond, p1) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1)) \
	requires (SENC_REQUIRES_CHECK(p1))

#define SENC_REQ_NOEXCEPT_1(p1) \
	noexcept(SENC_NOEXCEPT_CHECK(p1)) \
	requires (SENC_REQUIRES_CHECK(p1))

#define SENC_REQ_1(p1) \
	requires (SENC_REQUIRES_CHECK(p1))

#define SENC_REQ_NOEXCEPT_COND_2(cond, p1, p2) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2))

#define SENC_REQ_NOEXCEPT_2(p1, p2) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2))

#define SENC_REQ_2(p1, p2) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2))

#define SENC_REQ_NOEXCEPT_COND_3(cond, p1, p2, p3) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3))

#define SENC_REQ_NOEXCEPT_3(p1, p2, p3) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3))

#define SENC_REQ_3(p1, p2, p3) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3))

#define SENC_REQ_NOEXCEPT_COND_4(cond, p1, p2, p3, p4) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4))

#define SENC_REQ_NOEXCEPT_4(p1, p2, p3, p4) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4))

#define SENC_REQ_4(p1, p2, p3, p4) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4))

#define SENC_REQ_NOEXCEPT_COND_5(cond, p1, p2, p3, p4, p5) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5))

#define SENC_REQ_NOEXCEPT_5(p1, p2, p3, p4, p5) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5))

#define SENC_REQ_5(p1, p2, p3, p4, p5) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5))

#define SENC_REQ_NOEXCEPT_COND_6(cond, p1, p2, p3, p4, p5, p6) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6))

#define SENC_REQ_NOEXCEPT_6(p1, p2, p3, p4, p5, p6) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6))

#define SENC_REQ_6(p1, p2, p3, p4, p5, p6) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6))

#define SENC_REQ_NOEXCEPT_COND_7(cond, p1, p2, p3, p4, p5, p6, p7) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6) && SENC_NOEXCEPT_CHECK(p7)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7))

#define SENC_REQ_NOEXCEPT_7(p1, p2, p3, p4, p5, p6, p7) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6) && SENC_NOEXCEPT_CHECK(p7)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7))

#define SENC_REQ_7(p1, p2, p3, p4, p5, p6, p7) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7))

#define SENC_REQ_NOEXCEPT_COND_8(cond, p1, p2, p3, p4, p5, p6, p7, p8) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6) && SENC_NOEXCEPT_CHECK(p7) && SENC_NOEXCEPT_CHECK(p8)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8))

#define SENC_REQ_NOEXCEPT_8(p1, p2, p3, p4, p5, p6, p7, p8) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6) && SENC_NOEXCEPT_CHECK(p7) && SENC_NOEXCEPT_CHECK(p8)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8))

#define SENC_REQ_8(p1, p2, p3, p4, p5, p6, p7, p8) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8))

#define SENC_REQ_NOEXCEPT_COND_9(cond, p1, p2, p3, p4, p5, p6, p7, p8, p9) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6) && SENC_NOEXCEPT_CHECK(p7) && SENC_NOEXCEPT_CHECK(p8) && SENC_NOEXCEPT_CHECK(p9)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8) && SENC_REQUIRES_CHECK(p9))

#define SENC_REQ_NOEXCEPT_9(p1, p2, p3, p4, p5, p6, p7, p8, p9) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6) && SENC_NOEXCEPT_CHECK(p7) && SENC_NOEXCEPT_CHECK(p8) && SENC_NOEXCEPT_CHECK(p9)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8) && SENC_REQUIRES_CHECK(p9))

#define SENC_REQ_9(p1, p2, p3, p4, p5, p6, p7, p8, p9) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8) && SENC_REQUIRES_CHECK(p9))

#define SENC_REQ_NOEXCEPT_COND_10(cond, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) \
	noexcept((cond) && SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6) && SENC_NOEXCEPT_CHECK(p7) && SENC_NOEXCEPT_CHECK(p8) && SENC_NOEXCEPT_CHECK(p9) && SENC_NOEXCEPT_CHECK(p10)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8) && SENC_REQUIRES_CHECK(p9) && SENC_REQUIRES_CHECK(p10))

#define SENC_REQ_NOEXCEPT_10(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) \
	noexcept(SENC_NOEXCEPT_CHECK(p1) && SENC_NOEXCEPT_CHECK(p2) && SENC_NOEXCEPT_CHECK(p3) && SENC_NOEXCEPT_CHECK(p4) && SENC_NOEXCEPT_CHECK(p5) && SENC_NOEXCEPT_CHECK(p6) && SENC_NOEXCEPT_CHECK(p7) && SENC_NOEXCEPT_CHECK(p8) && SENC_NOEXCEPT_CHECK(p9) && SENC_NOEXCEPT_CHECK(p10)) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8) && SENC_REQUIRES_CHECK(p9) && SENC_REQUIRES_CHECK(p10))

#define SENC_REQ_10(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) \
	requires (SENC_REQUIRES_CHECK(p1) && SENC_REQUIRES_CHECK(p2) && SENC_REQUIRES_CHECK(p3) && SENC_REQUIRES_CHECK(p4) && SENC_REQUIRES_CHECK(p5) && SENC_REQUIRES_CHECK(p6) && SENC_REQUIRES_CHECK(p7) && SENC_REQUIRES_CHECK(p8) && SENC_REQUIRES_CHECK(p9) && SENC_REQUIRES_CHECK(p10))

// macro overload selector
#define GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NAME,...) NAME

// SENC_REQ_NOEXCEPT macro itself
#define SENC_REQ_NOEXCEPT(...) GET_MACRO(__VA_ARGS__, SENC_REQ_NOEXCEPT_10, SENC_REQ_NOEXCEPT_9, SENC_REQ_NOEXCEPT_8, SENC_REQ_NOEXCEPT_7, SENC_REQ_NOEXCEPT_6, SENC_REQ_NOEXCEPT_5, SENC_REQ_NOEXCEPT_4, SENC_REQ_NOEXCEPT_3, SENC_REQ_NOEXCEPT_2, SENC_REQ_NOEXCEPT_1)(__VA_ARGS__)

// SENC_REQ macro itself
#define SENC_REQ(...) GET_MACRO(__VA_ARGS__, SENC_REQ_10, SENC_REQ_9, SENC_REQ_8, SENC_REQ_7, SENC_REQ_6, SENC_REQ_5, SENC_REQ_4, SENC_REQ_3, SENC_REQ_2, SENC_REQ_1)(__VA_ARGS__)

// SENC_REQ_NOEXCEPT_COND is same as SENC_REQ_NOEXCEPT, but with preceeding noexcept condition
#define SENC_REQ_NOEXCEPT_COND(cond, ...) GET_MACRO(__VA_ARGS__, SENC_REQ_NOEXCEPT_COND_10, SENC_REQ_NOEXCEPT_COND_9, SENC_REQ_NOEXCEPT_COND_8, SENC_REQ_NOEXCEPT_COND_7, SENC_REQ_NOEXCEPT_COND_6, SENC_REQ_NOEXCEPT_COND_5, SENC_REQ_NOEXCEPT_COND_4, SENC_REQ_NOEXCEPT_COND_3, SENC_REQ_NOEXCEPT_COND_2, SENC_REQ_NOEXCEPT_COND_1)(cond, __VA_ARGS__)

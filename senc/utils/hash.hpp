/*********************************************************************
 * \file   hash.hpp
 * \brief  Contains hash-related utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include "concepts.hpp"

namespace senc::utils
{
	/**
	 * @class senc::utils::Hash
	 * @brief Custom hasher extending `std::hash`.
	 */
	template <typename T>
	class Hash { };

	/**
	 * @concept senc::utils::Hashable
	 * @brief Checks for a typename that can be hashed using `Senc::utils::Hash`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Hashable = requires(const Self self)
	{
		{ senc::utils::Hash<Self>{}(self) } -> ConvertibleTo<std::size_t>;
	};

	/**
	 * @concept senc::utils::Hashable
	 * @brief Checks for a typename that can be hashed using `Senc::utils::Hash`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HashableNoExcept = requires(const Self self)
	{
		{ senc::utils::Hash<Self>{}(self) } noexcept -> ConvertibleToNoExcept<std::size_t>;
	};

	/**
	 * @concept senc::utils::StdHashable
	 * @brief Checks for a typename that can be hashed using `std::hash`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept StdHashable = requires(const Self self)
	{
		{ std::hash<Self>{}(self) } -> ConvertibleTo<std::size_t>;
	};

	/**
	 * @concept senc::utils::StdHashable
	 * @brief Checks for a typename that can be hashed using `std::hash`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept StdHashableNoExcept = requires(const Self self)
	{
		{ std::hash<Self>{}(self) } noexcept -> ConvertibleToNoExcept<std::size_t>;
	};

	/**
	 * @brief Checks for a typename that can be hashed using a `hash` method.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasHashMethod = requires(const Self self)
	{
		{ self.hash() } -> ConvertibleTo<std::size_t>;
	};

	/**
	 * @brief Checks for a typename that can be hashed using a `hash` method, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HasHashMethodNoExcept = requires(const Self self)
	{
		{ self.hash() } noexcept -> ConvertibleToNoExcept<std::size_t>;
	};

	template <StdHashable T>
	class Hash<T>
	{
	public:
		std::size_t operator()(T value) const noexcept(StdHashableNoExcept<T>)
		{
			return std::hash<T>{}(value);
		}
	};

	template <HasHashMethod T>
	class Hash<T>
	{
	public:
		std::size_t operator()(T value) const noexcept(HasHashMethodNoExcept<T>)
		{
			return value.hash();
		}
	};

	template <Hashable... Ts>
	class Hash<std::tuple<Ts...>>
	{
	public:
		std::size_t operator()(const std::tuple<Ts...>& value) const
			noexcept((HashableNoExcept<Ts> && ...))
		{
			return hash_impl(std::index_sequence_for<Ts...>{});
		}

	private:
		template <std::size_t... Is>
		std::size_t hash_impl(std::index_sequence<Is...>)
		{
			return ((Hash<Ts>{}() << Is) ^ ...);
		}
	};

	template <Hashable K, typename V, Equaler<K> KeyEq = std::equal_to<K>>
	using HashMap = std::unordered_map<K, V, Hash<K>, KeyEq>;

	template <Hashable K, Equaler<K> KeyEq = std::equal_to<K>>
	using HashSet = std::unordered_set<K, Hash<K>, KeyEq>;
}

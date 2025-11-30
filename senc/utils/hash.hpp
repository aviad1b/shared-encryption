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
#include "concepts.hpp"
#include "uuid.hpp"

namespace senc::utils
{
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

	/**
	 * @concept senc::utils::Hashable
	 * @brief Checks for a typename that can be hashed using `Senc::utils::Hash`.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Hashable = HasHashMethod<Self> || StdHashable<Self>;

	/**
	 * @concept senc::utils::Hashable
	 * @brief Checks for a typename that can be hashed using `Senc::utils::Hash`, without throwing.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept HashableNoExcept = HasHashMethodNoExcept<Self> || StdHashableNoExcept<Self>;

	/**
	 * @class senc::utils::Hash
	 * @brief Custom hasher extending `std::hash`.
	 */
	template <Hashable T>
	class Hash
	{
	public:
		std::size_t operator()(T value) const noexcept(HashableNoExcept<T>)
		{
			if constexpr (HasHashMethod<T>)
				return value.hash();
			else
				return std::hash<T>{}(value);
		}
	};

	template <Hashable K, typename V, Equaler<K> KeyEq = std::equal_to<K>>
	using HashMap = std::unordered_map<K, V, Hash<K>, KeyEq>;

	template <Hashable K, Equaler<K> KeyEq = std::equal_to<K>>
	using HashSet = std::unordered_set<K, Hash<K>, KeyEq>;
}

/*********************************************************************
 * \file   hash.hpp
 * \brief  Contains hash-related utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

// this include is needed because CryptoPP uses WinAPI
#include "../utils/winapi_patch.hpp"

#include <cryptopp/sha.h>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include "concepts.hpp"
#include "bytes.hpp"
#include "math.hpp"

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

	template <>
	class Hash<BigInt>
	{
	public:
		std::size_t operator()(const BigInt& value) const
		{
			// serialize to big-endian byte array
			size_t n = value.MinEncodedSize();
			Buffer buff(n);
			value.Encode(buff.data(), buff.size());

			// hash bytes
			CryptoPP::SHA256 hash;
			byte digest[CryptoPP::SHA256::DIGESTSIZE];
			hash.CalculateDigest(digest, buff.data(), buff.size());

			// fold digest into size_t
			std::size_t result = 0;
			static_assert(sizeof(result) <= sizeof(digest));
			std::memcpy(&result, digest, sizeof(result));
			return result;
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
			return hash_impl(value, std::index_sequence_for<Ts...>{});
		}

	private:
		template <std::size_t... Is>
		std::size_t hash_impl(const std::tuple<Ts...>& value, std::index_sequence<Is...>) const
		{
			return ((Hash<Ts>{}(std::get<Is>(value)) << Is) ^ ...);
		}
	};

	/**
	 * @typedef senc::utils::HashMap
	 * @brief Hash (unordered) map which accepts key objects with a `hash` method..
	 * @tparam K Key type, must satisfy `senc::utils::Hashable`.
	 * @tparam V Value type.
	 * @tparam KeyEq Key comparing functor (e.g. `std::equal_to<K>`), must satisfy `senc::utils::Equaler<K>`.
	 */
	template <Hashable K, typename V, Equaler<K> KeyEq = std::equal_to<K>>
	using HashMap = std::unordered_map<K, V, Hash<K>, KeyEq>;

	/**
	 * @typedef senc::utils::HashSet
	 * @brief Hash (unordered) set which accepts key objects with a `hash` method..
	 * @tparam K Key type, must satisfy `senc::utils::Hashable`.
	 * @tparam KeyEq Key comparing functor (e.g. `std::equal_to<K>`), must satisfy `senc::utils::Equaler<K>`.
	 */
	template <Hashable K, Equaler<K> KeyEq = std::equal_to<K>>
	using HashSet = std::unordered_set<K, Hash<K>, KeyEq>;
}

/*********************************************************************
 * \file   uuid.hpp
 * \brief  Header of UUID utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "env.hpp"

#ifdef SENC_WINDOWS
#include <rpc.h>
#else
#include <uuid/uuid.h>
#endif

#include <ostream>
#include <string>
#include <array>
#include "Exception.hpp"
#include "concepts.hpp"
#include "bytes.hpp"
#include "hash.hpp"

namespace senc::utils
{
	/**
	 * @class senc::utils::UUIDException
	 * @brief Type of exceptions thrown on UUID related errors.
	 */
	class UUIDException : public Exception
	{
	public:
		using Self = UUIDException;
		using Base = Exception;

		UUIDException(const std::string& msg) : Base(msg) {}

		UUIDException(std::string&& msg) : Base(std::move(msg)) {}

		UUIDException(const std::string& msg, const std::string& info) : Base(msg, info) {}

		UUIDException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) {}

		UUIDException(const Self&) = default;

		Self& operator=(const Self&) = default;

		UUIDException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};

	/**
	 * @class senc::utils::UUID
	 * @brief Represents a unique ID (UUID).
	 */
	class UUID
	{
		static constexpr std::size_t SIZE = 16;

	public:
		using Self = UUID;
#ifdef SENC_WINDOWS
		using Underlying = ::UUID;
#else
		using Underlying = uuid_t;
#endif

		/**
		 * @brief Constructs a zero-value UUID.
		 */
		UUID() = default;

		/**
		 * @brief Constructs a UUID from a given string representation.
		 * @param value String representation of UUID value.
		 */
		UUID(const char* value);

		/**
		 * @brief Constructs a UUID from a given string representation.
		 * @param value String representation of UUID value.
		 */
		UUID(const std::string& value);

		/**
		 * @brief Copy constructor of UUID.
		 */
		UUID(const Self&) = default;

		/**
		 * @brief Copy assignment operator of UUID.
		 */
		Self& operator=(const Self&) = default;
		
		/**
		 * @brief Generates a (random) UUID.
		 * @return Generated UUID.
		 */
		static Self generate();

		/**
		 * @brief Generates a unique (random) UUID.
		 * @param existsPred A predicate function checking if UUID already exists.
		 * @return Generated UUID.
		 */
		static Self generate(Callable<bool, const Self&> auto&& existsPred)
			noexcept(CallableNoExcept<std::remove_cvref_t<decltype(existsPred)>, bool, const Self&>);

		/**
		 * @brief Generates a unique (random) UUID.
		 * @param container An object containing UUIDs, to check if already exists.
		 * @return Generated UUID.
		 */
		static Self generate(const HasContainsMethod<Self> auto& container)
			noexcept(HasContainsMethodNoExcept<std::remove_cvref_t<decltype(container)>, Self>);

		/**
		 * @brief Compares this UUID to another.
		 * @param other Other UUID to compare to.
		 * @return `true` if `*this` is the same UUID as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const noexcept;

		/**
		 * @brief Defines ordering of UUIDs (for ordered sets).
		 * @param other Other UUID to compare to.
		 * @return `true` if `*this` is ordered before `other`, otherwise `false`.
		 */
		bool operator<(const Self& other) const noexcept;

		/**
		 * @brief Gets string representation of UUID.
		 * @return String representation of UUID.
		 */
		std::string to_string() const;

		/**
		 * @brief Hashes UUID value.
		 * @return UUID hash.
		 */
		std::size_t hash() const noexcept;

		/**
		 * @brief Gets (byte) size of UUID value.
		 * @return Size of UUID value.
		 */
		static constexpr std::size_t size() { return SIZE; }

		/**
		 * @brief Gets pointer to byte data of UUID.
		 * @return Pointer to byte data of UUID.
		 */
		byte* data();

		/**
		 * @brief Gets pointer to byte data of UUID.
		 * @return Pointer to byte data of UUID.
		 */
		const byte* data() const;


		friend std::ostream& operator<<(std::ostream& os, const UUID& uuid);

	private:
		std::array<byte, SIZE> _bytes{};

		UUID(const Underlying& value);

		static void bytes_from_underlying(std::array<byte, SIZE>& out, const Underlying& underlying);

		static void underlying_from_bytes(Underlying& out, const std::array<byte, SIZE>& bytes);
	};
}

#include "uuid_impl.hpp"

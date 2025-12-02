/*********************************************************************
 * \file   uuid.hpp
 * \brief  Header of UUID utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <ostream>
#include <string>
#include <array>
#include <rpc.h>
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
	public:
		using Self = UUID;
		using Underlying = ::UUID;

		/**
		 * @brief Constructs a zero-value UUID.
		 */
		UUID();

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
		 * @brief Constructs a UUID from a given (moved) string representation.
		 * @param value String representation of UUID value (moved).
		 */
		UUID(std::string&& value);

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
		 * @brief Generates a (random) UUID.
		 * @param existsPred A predicate function checking if UUID already exists.
		 * @return Generated UUID.
		 */
		static Self generate(Callable<bool, const Self&> auto&& existsPred);

		/**
		 * @brief Compares this UUID to another.
		 * @param other Other UUID to compare to.
		 * @return `true` if `*this` is the same UUID as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const noexcept;

		/**
		 * @brief Gets string representation of UUID.
		 * @return String representation of UUID.
		 */
		const std::string& to_string() const noexcept;

		/**
		 * @brief Hashes UUID value.
		 * @return UUID hash.
		 */
		std::size_t hash() const noexcept;

		/**
		 * @brief Gets (byte) size of UUID value.
		 * @return Size of UUID value.
		 */
		constexpr std::size_t size() const { return 16; }

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
		std::array<byte, 16> _bytes{};
		std::string _str;

		UUID(const Underlying& value);

		static void bytes_from_underlying(std::array<byte, 16>& out, const Underlying& underlying);
	};
}

#include "uuid_impl.hpp"

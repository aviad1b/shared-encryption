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
#include <rpc.h>
#include "Exception.hpp"
#include "bytes.hpp"

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
		 * @brief Gets size of byte-serialized UUID.
		 * @return Size of byte-serialized UUID.
		 */
		static constexpr std::size_t bytes_size() { return 16; }

		/**
		 * @brief Serializes UUID into bytes.
		 * @return Serialized UUID.
		 */
		Buffer to_bytes() const;

		/**
		 * @brief Deserializes UUID from bytes.
		 * @param bytes Bytes to deserialize UUID from.
		 * @return Deserialized UUID.
		 */
		static Self from_bytes(const Buffer& bytes);


		friend std::ostream& operator<<(std::ostream& os, const UUID& uuid);

	private:
		Underlying _value{};
		std::string _str;

		UUID(const Underlying& value);
	};
}

/*********************************************************************
 * \file   BinFile.hpp
 * \brief  Header of BinFile class and utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "file_exceptions.hpp"
#include "FixedString.hpp"
#include "bytes.hpp"

namespace senc::utils
{
	/**
	 * @enum senc::utils::AccessFlags
	 * @brief Holds flag(s) of file access.
	 */
	enum class AccessFlags : std::uint8_t
	{
		Read   = 0b0001,
		Write  = 0b0010,
		Append = 0b0100,
		Edit   = 0b1000
	};

	/**
	 * @brief Combines access flags.
	 */
	inline constexpr AccessFlags operator|(AccessFlags a, AccessFlags b)
	{
		return static_cast<AccessFlags>(
			static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b)
		);
	}

	/**
	 * @brief Checks if second access flag(s) is included within first.
	 */
	inline constexpr bool operator&(AccessFlags a, AccessFlags b)
	{
		return static_cast<bool>(
			static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b)
		);
	}

	/**
	 * @brief Converts access flags to binary access mode.
	 */
	template <AccessFlags flags>
	inline constexpr FixedString<4> access_flags_to_binary_mode()
	{
		if constexpr (flags == AccessFlags::Read)
			return "rb\0";
		else if constexpr (flags == AccessFlags::Write)
			return "wb\0";
		else if constexpr (flags == AccessFlags::Append)
			return "ab\0";
		else if constexpr (flags == AccessFlags::Edit)
			return "rb+";
		else if constexpr (flags == (AccessFlags::Read | AccessFlags::Edit))
			return "rb+";
		else if constexpr (flags == (AccessFlags::Read | AccessFlags::Write))
			return "wb+";
		else if constexpr (flags == (AccessFlags::Read | AccessFlags::Append))
			return "ab+";
		else static_assert(false, "Unrecognized access flags");
	}

	/**
	 * @typedef senc::utils::file_pos_t
	 * @brief Represents cursor position in file.
	 */
	using file_pos_t = long;

	/**
	 * @class senc::utils::BinFile
	 * @brief Manages a binary file.
	 * @tparam accessFlags Flags determining access permissions.
	 * @tparam endianess Endianess used by the file.
	 */
	template <AccessFlags accessFlags, std::endian endianess = std::endian::little>
	class BinFile
	{
	public:
		using Self = BinFile<accessFlags>;

		/**
		 * @brief Opens file.
		 * @param path File path.
		 * @throw FileException If failed.
		 */
		BinFile(const std::string& path);

		/**
		 * @brief Closes file.
		 */
		~BinFile();

		/**
		 * @brief Move constructor of binary file.
		 */
		BinFile(Self&& other);

		/**
		 * @brief Move assignment operator of binary file.
		 * @note Used implicit move construction.
		 */
		Self& operator=(Self other);

		BinFile(const Self&) = delete;

		Self& operator=(const Self&) = delete;

		/**
		 * @brief Swaps with another instance.
		 */
		void swap(Self& other);

		/**
		 * @brief Gets the file's size.
		 * @return File's current size.
		 */
		file_pos_t size();

		/**
		 * @brief Gets current position in file.
		 * @return Current position in file.
		 * @throw FileException If failed.
		 */
		file_pos_t get_pos() const;

		/**
		 * @brief Set scurrent position in file.
		 * @param pos New position for file cursor.
		 * @throw FileException If failed.
		 */
		void set_pos(file_pos_t pos);

		/**
		 * @brief Reads data elements from the binary file.
		 * @tparam T Data element type.
		 * @param buffer Buffer to read data into.
		 * @param count Amount of element to read.
		 * @throw FileException If failed.
		 */
		template <std::integral T>
		void read(T* buffer, std::size_t count)
		requires ((accessFlags & AccessFlags::Read) || (accessFlags & AccessFlags::Edit));

		/**
		 * @brief Reads a single data element form the binary file.
		 * @tparam T Data element type (defaults to byte).
		 * @return Read element.
		 * @throw FileException If failed.
		 */
		template <std::integral T = byte>
		T read()
		requires ((accessFlags & AccessFlags::Read) || (accessFlags & AccessFlags::Edit));

		/**
		 * @brief Writes data elements to the binary file.
		 * @tparam T Data element type.
		 * @param buffer Data buffer to write from.
		 * @param count Element count to write.
		 * @throw FileException If failed.
		 */
		template <std::integral T>
		void write(const T* buffer, std::size_t count)
		requires ((accessFlags & (AccessFlags::Write)) ||
			(accessFlags & AccessFlags::Edit) ||
			(accessFlags & AccessFlags::Append));

		/**
		 * @brief Writes a single data element to the binary file.
		 * @tparam T Data element type.
		 * @param elem Data element to write.
		 * @throw FileException If failed.
		 */
		template <std::integral T>
		void write(T elem)
		requires ((accessFlags & (AccessFlags::Write)) ||
			(accessFlags & AccessFlags::Edit) ||
			(accessFlags & AccessFlags::Append));

		/**
		 * @brief Appends data elements to the binary file.
		 * @tparam T Data element type.
		 * @param buffer Data buffer to write from.
		 * @param count Element count to write.
		 * @throw FileException If failed.
		 */
		template <std::integral T>
		void append(const T* buffer, std::size_t count)
		requires ((accessFlags & (AccessFlags::Write)) ||
			(accessFlags & AccessFlags::Edit) ||
			(accessFlags & AccessFlags::Append));

		/**
		 * @brief Appends a single data element to the binary file.
		 * @tparam T Data element type.
		 * @param elem Data element to write.
		 * @throw FileException If failed.
		 */
		template <std::integral T>
		requires ((accessFlags & (AccessFlags::Write)) ||
			(accessFlags & AccessFlags::Edit) ||
			(accessFlags & AccessFlags::Append))
		void append(T elem);

	private:
		enum class UnderlyingOperation { None, Read, Write };

		std::FILE* _file;
		file_pos_t _pos;
		UnderlyingOperation _prevUnderlyingOperation;

		/**
		 * @brief Updates `_pos` field based on cursor.
		 */
		void update_internal_pos();

		/**
		 * @brief Utility function wrapping `fread`.
		 */
		template <std::integral T>
		void underlying_read(T* buffer, std::size_t count);

		/**
		 * @brief Utility function wrapping `fwrite`.
		 */
		template <std::integral T>
		void underlying_write(const T* buffer, std::size_t count);

		/**
		 * @brief Reverses endianess of all elements in a buffer.
		 */
		template <std::integral T>
		static void reverse_elems_endianess(T* buff, std::size_t count);
	};
}

#include "BinFile_impl.hpp"

/*********************************************************************
 * \file   BinFile_impl.hpp
 * \brief  Implementation of BinFile class and utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "BinFile.hpp"

#include "swap.hpp"

namespace senc::utils
{
	template <AccessFlags accessFlags, std::endian endianess>
	inline BinFile<accessFlags, endianess>::BinFile(std::string&& path)
		: _file(std::fopen(path.c_str(), access_flags_to_binary_mode<accessFlags>().c_str())),
		  _pos(0), _size(0), _prevUnderlyingOperation(UnderlyingOperation::None),
		  _path(std::move(path))
	{
		if (!_file)
			throw FileException("Failed to open file", this->_path);
		update_internal_pos_and_size();
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline BinFile<accessFlags, endianess>::BinFile(const std::string& path)
		: Self(std::string(path)) { }

	template <AccessFlags accessFlags, std::endian endianess>
	inline BinFile<accessFlags, endianess>::~BinFile()
	{
		if (this->_file)
			std::fclose(this->_file);
		this->_pos = 0;
		this->_size = 0;
		this->_prevUnderlyingOperation = UnderlyingOperation::None;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline BinFile<accessFlags, endianess>::BinFile(Self&& other) noexcept
		: _file(other._file), _pos(other._pos), _size(other._size),
		  _prevUnderlyingOperation(other._prevUnderlyingOperation),
		  _path(std::move(other._path))
	{
		other._file = nullptr;
		other._pos = 0;
		other._size = 0;
		other._prevUnderlyingOperation = UnderlyingOperation::None;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline BinFile<accessFlags, endianess>::Self&
		BinFile<accessFlags, endianess>::operator=(Self other)
	{
		utils::swap(*this, other);
		return *this;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline void BinFile<accessFlags, endianess>::swap(Self& other)
	{
		utils::swap(this->_file, other._file);
		utils::swap(this->_pos, other._pos);
		utils::swap(this->_size, other._size);
		utils::swap(this->_prevUnderlyingOperation, other._prevUnderlyingOperation);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline file_pos_t BinFile<accessFlags, endianess>::size() const
	{
		return _size;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline file_pos_t BinFile<accessFlags, endianess>::get_pos() const
	{
		return _pos;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline void BinFile<accessFlags, endianess>::set_pos(file_pos_t pos)
	{
		if (pos == _pos)
			return;
		underlying_seek(pos, SEEK_SET);
		_pos = pos;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	inline void BinFile<accessFlags, endianess>::read(T* buffer, std::size_t count)
	requires ((accessFlags & AccessFlags::Read) || (accessFlags & AccessFlags::Edit))
	{
		underlying_read(buffer, count);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <TupleSatisfies<std::is_integral> T>
	inline void BinFile<accessFlags, endianess>::read(T& out)
	requires ((accessFlags & AccessFlags::Read) || (accessFlags & AccessFlags::Edit))
	{
		std::apply([this](auto&... args)
		{
			(this->read(&args, 1), ...);
		}, out);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	inline T BinFile<accessFlags, endianess>::read()
	requires ((accessFlags & AccessFlags::Read) || (accessFlags & AccessFlags::Edit))
	{
		T res{};
		read(&res, 1);
		return res;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	inline void BinFile<accessFlags, endianess>::write(const T* buffer, std::size_t count)
	requires ((accessFlags & (AccessFlags::Write)) ||
		(accessFlags & AccessFlags::Edit) ||
		(accessFlags & AccessFlags::Append))
	{
		underlying_write(buffer, count);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <TupleSatisfies<std::is_integral> T>
	inline void BinFile<accessFlags, endianess>::write(T& out)
	requires ((accessFlags & (AccessFlags::Write)) ||
		(accessFlags & AccessFlags::Edit) ||
		(accessFlags & AccessFlags::Append))
	{
		std::apply([this](const auto&... args)
		{
			(this->write(&args, 1), ...);
		}, out);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	inline void senc::utils::BinFile<accessFlags, endianess>::write(T elem)
	requires ((accessFlags & (AccessFlags::Write)) ||
		(accessFlags & AccessFlags::Edit) ||
		(accessFlags & AccessFlags::Append))
	{
		write(&elem, 1);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	inline void BinFile<accessFlags, endianess>::append(const T* buffer, std::size_t count)
	requires ((accessFlags & (AccessFlags::Write)) ||
		(accessFlags & AccessFlags::Edit) ||
		(accessFlags & AccessFlags::Append))
	{
		underlying_seek(0, SEEK_END);
		underlying_write(buffer, count);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <TupleSatisfies<std::is_integral> T>
	inline void BinFile<accessFlags, endianess>::append(T& out)
	requires ((accessFlags & (AccessFlags::Write)) ||
		(accessFlags & AccessFlags::Edit) ||
		(accessFlags & AccessFlags::Append))
	{
		std::apply([this](const auto&... args)
		{
			(this->append(&args, 1), ...);
		}, out);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	requires ((accessFlags & (AccessFlags::Write)) ||
		(accessFlags & AccessFlags::Edit) ||
		(accessFlags & AccessFlags::Append))
	inline void BinFile<accessFlags, endianess>::append(T elem)
	{
		append(&elem, 1);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline void BinFile<accessFlags, endianess>::refresh_cursor()
	{
		underlying_seek(_pos, SEEK_SET);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline void BinFile<accessFlags, endianess>::update_internal_pos()
	{
		_pos = underlying_tell();
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline void BinFile<accessFlags, endianess>::update_internal_pos_and_size()
	{
		_pos = underlying_tell();
		underlying_seek(0, SEEK_END);
		_size = underlying_tell();
		underlying_seek(_pos, SEEK_SET);
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline void BinFile<accessFlags, endianess>::underlying_seek(file_pos_t pos, int origin)
	{
		if (0 != std::fseek(_file, pos, origin))
			throw FileException("Failed to set file position", this->_path);
		_prevUnderlyingOperation = UnderlyingOperation::None;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	inline file_pos_t BinFile<accessFlags, endianess>::underlying_tell()
	{
		file_pos_t pos = ftell(_file);
		if (pos < 0)
			throw FileException("Failed to locate file cursor", this->_path);
		return pos;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	inline void BinFile<accessFlags, endianess>::underlying_read(T* buffer, std::size_t count)
	{
		if (0 == count)
			return;

		// if prev operation was write, refresh cursor
		if (UnderlyingOperation::Write == _prevUnderlyingOperation)
			refresh_cursor();

		if (0 == std::fread(buffer, sizeof(T), count, _file))
			throw FileException("Failed to read from file", this->_path);

		// reverse endianess if needs to
		if constexpr (std::endian::native != endianess && sizeof(T) > 1)
			reverse_elems_endianess(buffer, count);
		update_internal_pos();
		_prevUnderlyingOperation = UnderlyingOperation::Read;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	inline void BinFile<accessFlags, endianess>::underlying_write(const T* buffer, std::size_t count)
	{
		if (0 == count)
			return;

		// if prev operation was read, refresh cursor
		if (UnderlyingOperation::Read == _prevUnderlyingOperation)
			refresh_cursor();

		if constexpr (std::endian::native == endianess || sizeof(T) <= 1)
		{
			if (0 == std::fwrite(buffer, sizeof(T), count, _file))
				throw FileException("Failed to write to file", this->_path);
		}
		else
		{
			std::vector<T> tempBuff(buffer, buffer + count);
			reverse_elems_endianess(tempBuff.data(), count);
			if (0 == std::fwrite(tempBuff.data(), sizeof(T), count, _file))
				throw FileException("Failed to write to file", this->_path);
		}
		update_internal_pos_and_size();
		_prevUnderlyingOperation = UnderlyingOperation::Write;
	}

	template <AccessFlags accessFlags, std::endian endianess>
	template <std::integral T>
	inline void BinFile<accessFlags, endianess>::reverse_elems_endianess(T* buff, std::size_t count)
	{
		for (std::size_t i = 0; i < count; ++i)
		{
			std::reverse(
				reinterpret_cast<byte*>(buff + i),
				reinterpret_cast<byte*>(buff + i + 1)
			);
		}
	}
}

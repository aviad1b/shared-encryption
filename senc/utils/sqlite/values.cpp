/*********************************************************************
 * \file   values.cpp
 * \brief  Implementation of sqlite values utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#include "values.hpp"

namespace senc::utils::sqlite
{
	ValueViewTag::ValueViewData::ValueViewData(sqlite3_value* value)
		: _data(value) { }

	ValueViewTag::ValueViewData::ValueViewData(sqlite3_stmt* stmt, int col)
		: _data(ColumnData{ stmt, col }) { }

	NullView::NullView(ValueViewData&& data) : _data(std::move(data))
	{
		if (SQLITE_NULL != _data.exec(sqlite3_value_type, sqlite3_column_type))
			throw SQLiteException("Bad null view handle");
	}

	NullView::NullView(sqlite3_value* value) : Self(ValueViewData(value)) { }

	NullView::NullView(sqlite3_stmt* stmt, int col) : Self(ValueViewData(stmt, col)) { }

	std::nullopt_t NullView::get() const
	{
		return std::nullopt;
	}

	NullView::operator std::nullopt_t() const
	{
		return get();
	}

	std::nullopt_t Null::get() const
	{
		return std::nullopt;
	}

	Null::operator std::nullopt_t() const
	{
		return get();
	}

	std::string Null::as_sqlite() const
	{
		return "NULL";
	}

	void Null::bind(sqlite3_stmt* stmt, int index) const
	{
		int code = sqlite3_bind_null(stmt, index);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to bind value", code);
	}

	IntView::IntView(ValueViewData&& data) : _data(std::move(data))
	{
		if (SQLITE_INTEGER != _data.exec(sqlite3_value_type, sqlite3_column_type))
			throw SQLiteException("Bad int view handle");
	}

	IntView::IntView(sqlite3_value* value) : Self(ValueViewData(value)) { }

	IntView::IntView(sqlite3_stmt* stmt, int col) : Self(ValueViewData(stmt, col)) { }

	std::int64_t IntView::get() const
	{
		return _data.exec(sqlite3_value_int64, sqlite3_column_int64);
	}

	IntView::operator std::int64_t() const
	{
		return get();
	}

	Int::Int() : Self(static_cast<std::int64_t>(0)) { }

	Int::Int(std::int64_t value) : _value(value) { }

	Int::Int(const View& view) : _value(view.get()) { }

	std::int64_t Int::get() const
	{
		return static_cast<std::int64_t>(_value);
	}

	Int::operator std::int64_t() const
	{
		return get();
	}

	std::string Int::as_sqlite() const
	{
		return std::to_string(_value);
	}

	void Int::bind(sqlite3_stmt* stmt, int index) const
	{
		int code = sqlite3_bind_int64(stmt, index, _value);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to bind value", code);
	}

	RealView::RealView(ValueViewData&& data) : _data(std::move(data))
	{
		if (SQLITE_FLOAT != _data.exec(sqlite3_value_type, sqlite3_column_type))
			throw SQLiteException("Bad real view handle");
	}

	RealView::RealView(sqlite3_value* value) : Self(ValueViewData(value)) { }

	RealView::RealView(sqlite3_stmt* stmt, int col) : Self(ValueViewData(stmt, col)) { }

	double RealView::get() const
	{
		return _data.exec(sqlite3_value_double, sqlite3_column_double);
	}

	RealView::operator double() const
	{
		return get();
	}

	Real::Real() : Self(0.0) { }

	Real::Real(double value) : _value(value) { }

	Real::Real(const View& view) : _value(view.get()) { }

	double Real::get() const
	{
		return _value;
	}

	Real::operator double() const
	{
		return get();
	}

	std::string Real::as_sqlite() const
	{
		return std::to_string(_value);
	}

	void Real::bind(sqlite3_stmt* stmt, int index) const
	{
		int code = sqlite3_bind_double(stmt, index, _value);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to bind value", code);
	}

	TextView::TextView(ValueViewData&& data) : _data(std::move(data))
	{
		if (SQLITE_TEXT != _data.exec(sqlite3_value_type, sqlite3_column_type))
			throw SQLiteException("Bad text view handle");
	}

	TextView::TextView(sqlite3_value* value) : Self(ValueViewData(value)) { }

	TextView::TextView(sqlite3_stmt* stmt, int col) : Self(ValueViewData(stmt, col)) { }

	std::string_view TextView::get() const
	{
		const char* ptr = reinterpret_cast<const char*>(
			_data.exec(sqlite3_value_text, sqlite3_column_text)
		);
		const int len = _data.exec(sqlite3_value_bytes, sqlite3_column_bytes);
		return { ptr, static_cast<size_t>(len) };
	}

	TextView::operator std::string_view() const
	{
		return get();
	}

	Text::Text(const std::string& value) : _value(value) { }

	Text::Text(std::string&& value) : _value(std::move(value)) { }

	Text::Text(const View& view) : _value(view.get().begin(), view.get().end()) { }

	const std::string& Text::get() const
	{
		return _value;
	}

	Text::operator const std::string&() const
	{
		return get();
	}

	std::string Text::as_sqlite() const
	{
		return "\"" + _value + "\"";
	}

	void Text::bind(sqlite3_stmt* stmt, int index) const
	{
		int code = sqlite3_bind_text(
			stmt, index,
			_value.c_str(), static_cast<int>(_value.length()),
			SQLITE_TRANSIENT // copy
		);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to bind value", code);
	}

	BlobView::BlobView(ValueViewData&& data) : _data(std::move(data))
	{
		if (SQLITE_BLOB != _data.exec(sqlite3_value_type, sqlite3_column_type))
			throw SQLiteException("Bad blob view handle");
	}

	BlobView::BlobView(sqlite3_value* value) : Self(ValueViewData(value)) { }

	BlobView::BlobView(sqlite3_stmt* stmt, int col) : Self(ValueViewData(stmt, col)) { }

	std::span<const byte> BlobView::get() const
	{
		const byte* ptr = reinterpret_cast<const byte*>(
			_data.exec(sqlite3_value_blob, sqlite3_column_blob)
		);
		const int len = _data.exec(sqlite3_value_bytes, sqlite3_column_bytes);
		return { ptr, static_cast<size_t>(len) };
	}

	BlobView::operator std::span<const byte>() const
	{
		return get();
	}

	Blob::Blob(Buffer&& value) : _value(std::move(value)) { }

	Blob::Blob(const View& view) : _value(view.get().begin(), view.get().end()) { }

	const Buffer& Blob::get() const
	{
		return _value;
	}

	Blob::operator const Buffer&() const
	{
		return get();
	}

	std::string Blob::as_sqlite() const
	{
		std::stringstream s;
		s << "x'";
		for (byte b : _value)
			s << std::hex << b;
		s << "'";
		return s.str();
	}

	void Blob::bind(sqlite3_stmt* stmt, int index) const
	{
		int code = sqlite3_bind_blob(
			stmt, index,
			_value.data(), static_cast<int>(_value.size()),
			SQLITE_TRANSIENT // copy
		);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to bind value");
	}
}

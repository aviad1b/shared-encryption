/*********************************************************************
 * \file   Database_impl.hpp
 * \brief  Implementation of sqlite Database class.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#include "Database.hpp"

#include "sqlite_utils.hpp"
#include <cstdio>

namespace senc::utils::sqlite
{
	template <schemas::SomeTable... Ts>
	class DatabaseUtils;

	template <FixedString name, schemas::SomeCol... Cs>
	class TableUtils;

	template <schemas::SomeCol C>
	class ColUtils;

	template <schemas::SomeDB Schema>
	inline Database<Schema>::Database(const std::string& path)
		: _path(path), _db(nullptr)
	{
		int code = sqlite3_open(_path.c_str(), &_db);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to open database " + _path, code);

		DatabaseUtils(Schema{}).create_tables_if_not_exist(_db);
	}

	template <schemas::SomeDB Schema>
	inline Database<Schema>::~Database()
	{
		if (_db)
		{
			sqlite3_close(_db);
			_db = nullptr;
		}
	}

	template <schemas::SomeDB Schema>
	template <FixedString tableName, Param... Values>
	requires schemas::PARAMS_FOR_TABLE<schemas::DBTable<Schema, tableName>, Values...>
	inline void Database<Schema>::insert(Values&&... values)
	{
		using T = schemas::DBTable<Schema, tableName>;
		constexpr std::size_t COLS_COUNT = std::tuple_size_v<schemas::TableTuple<T>>;
		std::string sql = "INSERT INTO " + std::string(schemas::TABLE_NAME<T>) +
			"(" + TableUtils(T{}).get_columns() + ") VALUES(";
		if (COLS_COUNT > 0)
		{
			for (std::size_t i = 0; i < COLS_COUNT - 1; ++i)
				sql += "?, ";
			sql += "?";
		}
		sql += ");";

		sqlite3_stmt* stmt = nullptr;
		int code = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to run statement: " + sql, code);

		// bind parameters
		ParamUtils::bind_all(
			std::make_index_sequence<sizeof...(Values)>{},
			stmt, values...
		);

		code = sqlite3_step(stmt);
		if (SQLITE_DONE != code)
			throw SQLiteException("Failed to insert into table " + std::string(tableName), code);

		// cleanup
		sqlite3_finalize(stmt);
	}

	template <schemas::SomeDB Schema>
	template <FixedString tableName>
	requires schemas::DBWithTable<Schema, tableName>
	inline void Database<Schema>::remove(const std::string& where)
	{
		const std::string sql = "DELETE FROM " + std::string(tableName) +
			" WHERE " + where;
		int code = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, nullptr);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to remove", code);
	}

	template <schemas::SomeDB Schema>
	template <FixedString tableName, SomeSelectArg... Args>
	requires schemas::Selectable<
		schemas::DBTable<Schema, tableName>,
		Args...
	>
	inline TableView<schemas::Select<
		schemas::DBTable<Schema, tableName>,
		Args...
	>> Database<Schema>::select()
	{
		return TableView<schemas::Select<
			schemas::DBTable<Schema, tableName>,
			Args...
		>>(_db);
	}

	template <schemas::SomeDB Schema>
	template <FixedString table1Name, FixedString table2Name, FixedString axisCol>
	requires schemas::Joinable<
		schemas::DBTable<Schema, table1Name>,
		schemas::DBTable<Schema, table2Name>,
		axisCol
	>
	inline TableView<schemas::Join<
		schemas::DBTable<Schema, table1Name>,
		schemas::DBTable<Schema, table2Name>,
		axisCol
	>> Database<Schema>::join()
	{
		return TableView<schemas::Join<
			schemas::DBTable<Schema, table1Name>,
			schemas::DBTable<Schema, table2Name>,
			axisCol
		>>(_db);
	}

	template <schemas::SomeDB Schema>
	inline TempDatabase<Schema>::TempDatabase(const std::string& path)
		: Base(path) { }

	template <schemas::SomeDB Schema>
	inline TempDatabase<Schema>::~TempDatabase()
	{
		if (this->_db)
		{
			sqlite3_close(this->_db);
			this->_db = nullptr;
		}
		if (!this->_path.empty())
			std::remove(this->_path.c_str());
	}
}

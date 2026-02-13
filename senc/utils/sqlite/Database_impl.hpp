/*********************************************************************
 * \file   Database_impl.hpp
 * \brief  Implementation of sqlite Database class.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#include "Database.hpp"
#include "sqlite_utils.hpp"

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
		: _db(nullptr)
	{
		if (SQLITE_OK != sqlite3_open(path.c_str(), &_db))
			throw SQLiteException("Failed to open database " + path);

		TableUtils(Schema{}).create_tables_if_not_exist(_db);
	}

	template <schemas::SomeDB Schema>
	inline Database<Schema>::~Database()
	{
		sqlite3_close(_db);
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
		if (SQLITE_OK != sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr))
			throw SQLiteException("Failed to run statement: " + sql);

		// bind parameters
		ParamUtils::bind_all(
			std::make_index_sequence<sizeof...(Values)>{},
			stmt, values...
		);

		if (SQLITE_DONE != sqlite3_step(stmt))
			throw SQLiteException("Failed to insert into table " + std::string(tableName));

		// cleanup
		sqlite3_finalize(stmt);
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
}

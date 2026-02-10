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

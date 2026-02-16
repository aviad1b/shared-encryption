/*********************************************************************
 * \file   Database.hpp
 * \brief  Header of sqlite Database class.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "TableView.hpp"

namespace senc::utils::sqlite
{
	/**
	 * @class senc::utils::sqlite::Database
	 * @brief Manages an SQLite database.
	 * @tparam Schema Database schema.
	 * @note Has forward declaration in `sqlite_utils.hpp`
	 */
	template <schemas::SomeDB Schema>
	class Database
	{
	public:
		using Self = Database<Schema>;

		/**
		 * @brief Loads database from file.
		 * @param path Database file path.
		 * @throw SQLiteException If failed to open database.
		 */
		Database(const std::string& path);

		/**
		 * @brief Database destructor, closes database.
		 */
		virtual ~Database();

		/**
		 * @brief Inserts a record into a table of the database.
		 * @tparam tableName Name of table to insert into.
		 * @param values Values to insert.
		 * @throw SQLiteException If insertion failed.
		 */
		template <FixedString tableName, Param... Values>
		requires schemas::PARAMS_FOR_TABLE<schemas::DBTable<Schema, tableName>, Values...>
		void insert(Values&&... values);

		/**
		 * @brief Removes (a) record(s) from a table of the database.
		 * @tparam tableName Name of table to remove from.
		 * @param where Where clause to locate records to remove.
		 * @throw SQLiteException If removal failed.
		 */
		template <FixedString tableName>
		requires schemas::DBWithTable<Schema, tableName>
		void remove(const std::string& where);

		/**
		 * @brief Applies "select" on database (and gets fitting table view).
		 * @tparam Args Select arguments.
		 * @see senc::utils::sqlite::SelectArg
		 * @return Table view with "select" applied.
		 */
		template <FixedString tableName, SomeSelectArg... Args>
		requires schemas::Selectable<
			schemas::DBTable<Schema, tableName>,
			Args...
		>
		TableView<schemas::Select<
			schemas::DBTable<Schema, tableName>,
			Args...
		>> select();

		/**
		 * @brief Applies (inner) join on database (and gets fitting table view).
		 * @tparam table1Name Name of first table to join.
		 * @tparam axisCol1 Name of column (in table1) to join on.
		 * @tparam table2Name Name of second table to join.
		 * @tparam axisCol2 Name of column in table2 to join on (defaults to axisCol1).
		 * @return Table view with "join" applied.
		 */
		template <FixedString table1Name, FixedString axisCol1,
			FixedString table2Name, FixedString axisCol2 = axisCol1>
		requires schemas::Joinable<
			schemas::DBTable<Schema, table1Name>,
			axisCol1,
			schemas::DBTable<Schema, table2Name>,
			axisCol2
		>
		TableView<schemas::Join<
			schemas::DBTable<Schema, table1Name>,
			axisCol1,
			schemas::DBTable<Schema, table2Name>,
			axisCol2
		>> join();

	protected:
		std::string _path;
		sqlite3* _db;
	};

	/**
	 * @class senc::utils::sqlite::TempDatabase
	 * @brief Manages a temporary SQLite database.
	 * @tparam Schema Database schema.
	 */
	template <schemas::SomeDB Schema>
	class TempDatabase : public Database<Schema>
	{
	public:
		using Self = TempDatabase<Schema>;
		using Base = Database<Schema>;

		/**
		 * @brief Loads database from file.
		 * @param path Database file path.
		 * @throw SQLiteException If failed to open database.
		 */
		TempDatabase(const std::string& path);

		/**
		 * @brief Database destructor, closes & deletes database.
		 */
		virtual ~TempDatabase();
	};
}

#include "Database_impl.hpp"

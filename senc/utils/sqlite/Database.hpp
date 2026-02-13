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
		~Database();

		/**
		 * @brief Inserts a record in to a table of the database.
		 * @tparam tableName Name of table to insert into.
		 * @param values Values to insert.
		 * @throw SQLiteException If insertion failed.
		 */
		template <FixedString tableName, Param... Values>
		requires schemas::PARAMS_FOR_TABLE<schemas::DBTable<Schema, tableName>, Values...>
		void insert(Values&&... values);

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
		 * @tparam table2Name Name of second table to join.
		 * @tparam axisCol Name of column to join on.
		 * @return Table view with "join" applied.
		 */
		template <FixedString table1Name, FixedString table2Name, FixedString axisCol>
		requires schemas::Joinable<
			schemas::DBTable<Schema, table1Name>,
			schemas::DBTable<Schema, table2Name>,
			axisCol
		>
		TableView<schemas::Join<
			schemas::DBTable<Schema, table1Name>,
			schemas::DBTable<Schema, table2Name>,
			axisCol
		>> join();

	protected:
		/**
		 * @brief Gets database file path.
		 * @return Database file path.
		 */
		const std::string& path() const;

	private:
		std::string _path;
		sqlite3* _db;
	};
}

#include "Database_impl.hpp"

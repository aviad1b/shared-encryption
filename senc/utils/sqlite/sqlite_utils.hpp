/*********************************************************************
 * \file   sqlite_utils.hpp
 * \brief  Header of utilities for sqlite classes.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "TableView.hpp"
#include "Database.hpp"

namespace senc::utils::sqlite
{
	template <schemas::SomeDB Schema>
	class Database;

	template <schemas::SomeTable Schema>
	class TableView;

	template <schemas::SomeTable... Ts>
	class DatabaseUtils
	{
		using Schema = schemas::DB<Ts...>;
		friend class Database<Schema>;

		// dummy arg is used for template inference
		DatabaseUtils(Schema) { }

		/**
		 * @brief Creates each table in schema if doesn't exist.
		 * @param db Database handle pointer.
		 * @throw SQLiteException On error.
		 */
		void create_tables_if_not_exist(sqlite3* db);
	};

	template <FixedString name, schemas::SomeCol... Cs>
	class TableUtils
	{
		using Schema = schemas::Table<name, Cs...>;
		friend class TableView<Schema>;

		template <schemas::SomeTable... Ts>
		friend class DatabaseUtils;

		template <schemas::SomeDB Schema>
		friend class Database;

		// dummy arg is used for template inference
		TableUtils(Schema) { }

		/**
		 * @brief Executes a statement with a given callback function on a table.
		 * @param db Database handle pointer.
		 * @param sql SQL statement to run.
		 * @param callback A callback function (of fitting schema).
		 * @param limit Optional expected record limit.
		 * @throw SQLiteException If `limit` was provided and exceeded.
		 */
		void execute(sqlite3* db,
					 const std::string& sql,
					 schemas::TableCallable<Schema> auto&& callback,
					 std::optional<int> limit);

		/**
		 * @brief utility function for `execute`.
		 * @tparam is Index sequence for columns.
		 * @param callback Callback function (from `execute`).
		 * @param stmt Statement to run callback on.
		 */
		template <std::size_t... is>
		static void execute_util(
			schemas::TableCallable<schemas::Table<name, Cs...>> auto&& callback,
			sqlite3_stmt* stmt);

		/**
		 * @brief Gets SQL create statement for table.
		 * @return SQL create statement.
		 */
		static std::string get_create_statement();

		/**
		 * @brief Gets columns of table in one string.
		 * @return Columns of table in one string.
		 */
		static std::string get_columns();
	};

	template <schemas::SomeCol C>
	class ColUtils
	{
		template <FixedString name, schemas::SomeCol... Cs>
		friend class TableUtils;

		// dummy arg is used for template inference
		ColUtils(C) { }

		/**
		 * @brief Gets SQL create statement arg for column.
		 * @return SQL create statement arg (including comma).
		 */
		static std::string get_create_arg();

		/**
		 * @brief Gets SQL additional constraints for column.
		 * @return SQL constraints (separated by comma, ending with comma).
		 */
		static std::string get_additional_constraints();
	};

	class ParamUtils
	{
		template <schemas::SomeDB Schema>
		friend class Database;

		/**
		 * @brief Binds a value to a statement parameter.
		 * @tparam P Parameter type.
		 * @tparam i Statement param index.
		 * @param stmt Statement handle pointer.
		 * @param value Value to bind.
		 * @throw SQLiteException If failed to bind.
		 */
		template <std::size_t i, typename P>
		static void bind_one(sqlite3_stmt* stmt, const P& value);

		/**
		 * @brief Binds values to a statement parameter.
		 * @tparam Ps Parameter types.
		 * @tparam is Statement param indexes.
		 * @param stmt Statement handle pointer.
		 * @param values Values to bind.
		 * @throw SQLiteException If failed to bind.
		 */
		template <std::size_t... is, typename... Ps>
		static void bind_all(std::index_sequence<is...> dummy, sqlite3_stmt* stmt, const Ps&... values);
	};
}

#include "sqlite_utils_impl.hpp"

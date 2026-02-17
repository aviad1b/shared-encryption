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
#include "args.hpp"

namespace senc::utils::sqlite
{
	template <schemas::SomeDB Schema>
	class Database;

	template <schemas::SomeTable Schema>
	class TableView;

	/**
	 * @class senc::utils::sqlite::ParamUtils
	 * @brief Contains private utility parameter functions.
	 */
	class ParamUtils
	{
		template <schemas::SomeDB Schema>
		friend class Database;

		/**
		 * @brief Binds a parameter to a statement parameter.
		 * @tparam i Statement param index.
		 * @tparam P Parameter type.
		 * @param stmt Statement handle pointer.
		 * @param param Parameter to bind.
		 * @throw SQLiteException If failed to bind.
		 */
		template <std::size_t i, Param P>
		static void bind_one(sqlite3_stmt* stmt, const P& param);

		/**
		 * @brief Binds parameters to a statement parameter.
		 * @tparam is Statement param indexes.
		 * @tparam Ps Parameter types.
		 * @param stmt Statement handle pointer.
		 * @param params Parameters to bind.
		 * @throw SQLiteException If failed to bind.
		 */
		template <std::size_t... is, Param... Ps>
		static void bind_all(std::index_sequence<is...> dummy, sqlite3_stmt* stmt, const Ps&... params);
	};

	/**
	 * @class senc::utils::sqlite::ColUtils
	 * @brief Contains private utility column functions.
	 * @tparam C Column schema.
	 */
	template <schemas::SomeCol C>
	class ColUtils
	{
		template <FixedString name, schemas::SomeCol... Cs>
		friend class TableUtils;

		// dummy arg is used for template inference
		constexpr ColUtils(C) { }

		/**
		 * @brief Gets SQL create statement arg for column.
		 * @return SQL create statement arg.
		 */
		static constexpr auto get_create_arg()
		{
			if constexpr (schemas::SomePrimaryKey<C>)
				return schemas::COL_NAME<C> + " " +
					schemas::COL_SQL_TYPE<C> + " PRIMARY KEY";
			else if constexpr (schemas::ColType<C>::is_nullable())
				return schemas::COL_NAME<C> + " " +
					schemas::COL_SQL_TYPE<C>;
			else
				return schemas::COL_NAME<C> + " " +
					schemas::COL_SQL_TYPE<C> + " NOT NULL";
		}

		/**
		 * @brief Gets SQL additional constraints for column.
		 * @return SQL constraints (separated by comma).
		 */
		static constexpr auto get_additional_constraints()
		{
			constexpr auto foreignKeyRet =
				"FOREIGN KEY (" + schemas::COL_NAME<C> +") REFERENCES " +
				schemas::FOREIGN_KEY_REF_TABLE_NAME<C> +
				"(" + schemas::FOREIGN_KEY_REF_COL_NAME<C> +
				") ON DELETE CASCADE ON UPDATE NO ACTION";
			constexpr std::size_t n = foreignKeyRet.len() + 1;
			if constexpr (schemas::SomeForeignKey<C>)
				return FixedString<n>(foreignKeyRet);
			else return FixedString<n>();
		}
	};

	/**
	 * @class senc::utils::sqlite::TableUtils
	 * @brief Contains private utility table functions.
	 * @tparam name Table name.
	 * @tparam Cs Table columns (schemas).
	 */
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
		constexpr TableUtils(Schema) { }

		/**
		 * @brief Executes a statement with a given callback function on a table.
		 * @param db Database handle pointer.
		 * @param sql SQL statement to run.
		 * @param callback A callback function (of fitting schema).
		 * @param expected Optional expected record count.
		 * @throw SQLiteException If `expected` was provided and exceeded or not met.
		 */
		void execute(sqlite3* db,
					 const std::string& sql,
					 schemas::TableCallable<Schema> auto&& callback,
					 std::optional<int> expected);

		/**
		 * @brief utility function for `execute`.
		 * @tparam is Index sequence for columns.
		 * @param callback Callback function (from `execute`).
		 * @param stmt Statement to run callback on.
		 */
		template <std::size_t... is>
		static void execute_util(
			std::index_sequence<is...>,
			schemas::TableCallable<schemas::Table<name, Cs...>> auto&& callback,
			sqlite3_stmt* stmt)
		{
			// for each column C with index i,
			// construct a view of that column from stmt and i
			callback(schemas::ColView<Cs>(stmt, is)...);
			// NOTE: clang requires this to be defined here (and not in impl)
		}

		/**
		 * @brief Gets SQL create statement for table.
		 * @return SQL create statement.
		 */
		static constexpr auto get_create_statement()
		{
			return "CREATE TABLE IF NOT EXISTS " +
				name + "(" +
				FIXED_STRING_JOIN<
					",",
					ColUtils(Cs{}).get_create_arg()...,
					ColUtils(Cs{}).get_additional_constraints()...
				> + ");";
		}

		/**
		 * @brief Gets columns of table in one string.
		 * @return Columns of table in one string.
		 */
		static constexpr auto get_columns()
		{
			return FIXED_STRING_JOIN<",", schemas::COL_NAME<Cs>...>;
		}
	};

	/**
	 * @class senc::utils::sqlite::DatabaseUtils
	 * @brief Contains private utility database functions.
	 * @tparam Ts Database table schemas.
	 */
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
}

#include "sqlite_utils_impl.hpp"

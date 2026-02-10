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
	template <FixedString name, schemas::SomeCol... Cs>
	class TableUtils
	{
		using Schema = schemas::Table<name, Cs...>;
		friend class TableView<Schema>;

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
	};
}

#include "sqlite_utils_impl.hpp"

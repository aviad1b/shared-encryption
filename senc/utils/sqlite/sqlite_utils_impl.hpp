/*********************************************************************
 * \file   sqlite_utils_impl.hpp
 * \brief  Implementation of utilities for sqlite classes.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#include "sqlite_utils.hpp"

namespace senc::utils::sqlite
{
	template <FixedString name, schemas::SomeCol... Cs>
	inline void TableUtils<name, Cs...>::execute(
		sqlite3* db,
		const std::string& sql,
		schemas::TableCallable<Schema> auto&& callback,
		std::optional<int> limit)
	{
		sqlite3_stmt* stmt = nullptr;

		if (SQLITE_OK != sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr))
			throw SQLiteException("Failed to run statement: " + sql);

		// cleanup of `stmt` at scope exit
		AtScopeExit cleanup([stmt]() { sqlite3_finalize(stmt); });

		// if has limit, set limit function to compare; otherwise, limit function always false
		auto pastLimit = limit.has_value() ? [limit](int i) { return i >= *limit; }
		: [](int) { return false; };

		for (int i = 0; SQLITE_ROW == sqlite3_step(stmt); ++i)
		{
			if (pastLimit(i))
				throw SQLiteException("Too many rows to unpack: Expected " + std::to_string(*limit));

			execute_util<std::make_index_sequence<sizeof...(Cs)>>(
				callback, stmt
			);
		}
	}

	template <FixedString name, schemas::SomeCol... Cs>
	template <std::size_t... is>
	inline void TableUtils<name, Cs...>::execute_util(
		schemas::TableCallable<schemas::Table<name, Cs...>> auto&& callback,
		sqlite3_stmt* stmt)
	{
		// for each column C with index i,
		// construct a view of that column from stmt and i
		callback(schemas::ColView<Cs>(stmt, is)...);
	}
}

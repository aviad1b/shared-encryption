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
	template <std::size_t i, Param P>
	inline void ParamUtils::bind_one(sqlite3_stmt* stmt, const P& param)
	{
		constexpr int index = static_cast<int>(i) + 1; // sql starts counting params from one
		param.bind(stmt, index);
	}

	template <std::size_t... is, Param... Ps>
	inline void ParamUtils::bind_all(std::index_sequence<is...> dummy, sqlite3_stmt* stmt, const Ps&... params)
	{
		(void)dummy; // for template inference
		(bind_one<is, Ps>(stmt, params), ...);
	}

	template <FixedString name, schemas::SomeCol... Cs>
	inline void TableUtils<name, Cs...>::apply_backwards_compatibility(sqlite3* db)
	{
		apply_backwards_compatibility_util(
			std::make_index_sequence<sizeof...(Cs)>{},
			db
		);
	}

	template <FixedString name, schemas::SomeCol ...Cs>
	template <std::size_t ...is>
	inline void TableUtils<name, Cs...>::apply_backwards_compatibility_util(
		std::index_sequence<is...>, sqlite3* db)
	{
		// for each column C with index i,
		// if column is backwards compatible and doesn't exist, create it
		auto callback = [db](auto c, std::size_t i)
		{
			using C = decltype(c);
			if constexpr (!schemas::SomeBackwardsCompatible<C>)
				return;

			constexpr auto COL_CHECK = "PRAGMA table_info(" + name + ");";

			constexpr auto COL_CREATE = "ALTER TABLE " + name +
				" ADD COLUMN " + ColUtils<C>::get_create_arg() + " DEFAULT NULL;";

			sqlite3_stmt* stmt = nullptr;
			sqlite3_prepare_v2(db, COL_CHECK.c_str(), -1, &stmt, nullptr);

			bool colExists = false;
			while (SQLITE_ROW == sqlite3_step(stmt) && !colExists)
			{
				const char* col = reinterpret_cast<const char*>(
					sqlite3_column_text(stmt, static_cast<int>(i) + 1) // col index starts at 1
				);
				if (col && 0 == std::strcmp(col, schemas::COL_NAME<C>.c_str()))
					colExists = true;
			}

			sqlite3_finalize(stmt);

			if (!colExists)
				sqlite3_exec(db, COL_CREATE.c_str(), nullptr, nullptr, nullptr);
		};
		(callback(Cs{}, is), ...);
		// NOTE: clang requires this to be defined here (and not in impl)
	}

	template <FixedString name, schemas::SomeCol... Cs>
	inline void TableUtils<name, Cs...>::execute(
		sqlite3* db,
		const std::string& sql,
		schemas::TableCallable<Schema> auto&& callback,
		std::optional<int> expected)
	{
		sqlite3_stmt* stmt = nullptr;

		int code = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to run statement: " + sql, code);

		// cleanup of `stmt` at scope exit
		AtScopeExit cleanup([stmt]() { sqlite3_finalize(stmt); });

		// if has limit, set limit function to compare; otherwise, limit function always false
		std::function<bool(int)> pastLimit = expected.has_value()
			? std::function<bool(int)>{ [expected](int i) { return i >= *expected; } }
			: std::function<bool(int)>{ [](int) { return false; } };

		int i = 0;
		for (; SQLITE_ROW == sqlite3_step(stmt); ++i)
		{
			if (pastLimit(i))
				throw SQLiteException("Too many rows to unpack: Expected " + std::to_string(*expected));

			execute_util(
				std::make_index_sequence<sizeof...(Cs)>{},
				callback, stmt
			);
		}

		if (expected.has_value() && i < *expected)
			throw SQLiteException("Too few rows to unpack: Expected " + std::to_string(*expected));
	}

	template <schemas::SomeTable... Ts>
	inline void DatabaseUtils<Ts...>::create_tables_if_not_exist(sqlite3* db)
	{
		const auto sql = "BEGIN; " +
			((TableUtils(Ts{}).get_create_statement() + " ") + ...) +
			"COMMIT;";
		int code = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to create tables", code);

		// apply backwards compatibility on all tables
		((TableUtils(Ts{}).apply_backwards_compatibility(db)), ...);
	}
}

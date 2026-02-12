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
	template <schemas::SomeTable... Ts>
	inline void DatabaseUtils<Ts...>::create_tables_if_not_exist(sqlite3* db)
	{
		const std::string sql = "BEGIN;" +
			(TableUtils(Ts{}).get_create_statement() + ...) +
			"COMMIT;";
		if (SQLITE_OK != sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr))
			throw SQLiteException("Failed to create tables");
	}

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
	inline std::string TableUtils<name, Cs...>::get_create_statement()
	{
		std::string res = std::string("CREATE TABLE IF NOT EXISTS ") +
			name + "(" +
			(ColUtils(Cs{}).get_create_arg() + ...) +
			(ColUtils(Cs{}).get_additional_constraints() + ...);

		// remove last comma (and add remaining of statement)
		if (res.ends_with(","))
			res = res.substr(0, res.length() - 1);
		res += ");";
		return res;
	}

	template <FixedString name, schemas::SomeCol... Cs>
	inline std::string TableUtils<name, Cs...>::get_columns()
	{
		std::string res = ((std::string(schemas::COL_NAME<Cs>.view()) + ",") + ...);
		if (res.ends_with(","))
			res = res.substr(0, res.length() - 1);
		return res;
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

	template <schemas::SomeCol C>
	inline std::string ColUtils<C>::get_create_arg()
	{
		if constexpr (schemas::SomePrimaryKey<C>)
			return schemas::COL_NAME<C> + " " + schemas::COL_SQL_TYPE<C> + "PRIMARY KEY,";
		else if constexpr (schemas::ColType<C>::is_nullable())
			return schemas::COL_NAME<C> + " " + schemas::COL_SQL_TYPE<C> + ",";
		else
			return schemas::COL_NAME<C> + " " + schemas::COL_SQL_TYPE<C> + " NOT NULL,";
	}

	template <schemas::SomeCol C>
	inline std::string ColUtils<C>::get_additional_constraints()
	{
		if constexpr (schemas::SomeForeignKey<C>)
			return "FOREIGN KEY (" + schemas::COL_NAME<C> + ") REFERENCES " +
				schemas::FOREIGN_KEY_REF_TABLE_NAME<C> +
				"(" + schemas::FOREIGN_KEY_REF_COL_NAME<C> +
				") ON DELETE CASCADE ON UPDATE NO ACTION,";
		else return "";
	}

	template <std::size_t i, Param P>
	inline void ParamUtils::bind_one(sqlite3_stmt* stmt, const P& value)
	{
		constexpr int index = static_cast<int>(i);
		int status = 0;
		if constexpr (OneOf<P, std::nullptr_t, std::nullopt_t>)
			status = sqlite3_bind_null(stmt, index);
		else if constexpr (std::same_as<P, std::int64_t>)
			status = sqlite3_bind_int64(stmt, index, value);
		else if constexpr (std::same_as<P, double>)
			status = sqlite3_bind_double(stmt, index, value);
		else if constexpr (std::same_as<P, std::string>)
			status = sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_STATIC);
		else // buffer
			status = sqlite3_bind_blob(stmt, index, value.data(), static_cast<int>(value.size()), SQLITE_STATIC);

		if (SQLITE_OK != status)
			throw SQLiteException("Failed to bind parameter");
	}

	template <std::size_t... is, Param... Ps>
	inline void ParamUtils::bind_all(std::index_sequence<is...> dummy, sqlite3_stmt* stmt, const Ps&... values)
	{
		(void)dummy; // for template inference
		(bind_one<is, Ps>(stmt, values), ...);
	}
}

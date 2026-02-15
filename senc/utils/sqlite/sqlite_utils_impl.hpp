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
		int code = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
		if (SQLITE_OK != code)
			throw SQLiteException("Failed to create tables", code);
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

	template <FixedString name, schemas::SomeCol... Cs>
	inline std::string TableUtils<name, Cs...>::get_create_statement()
	{
		std::string res = "CREATE TABLE IF NOT EXISTS " +
			std::string(name) + "(" +
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

	template <schemas::SomeCol C>
	inline std::string ColUtils<C>::get_create_arg()
	{
		if constexpr (schemas::SomePrimaryKey<C>)
			return std::string(schemas::COL_NAME<C>) + " " +
				std::string(schemas::COL_SQL_TYPE<C>) + "PRIMARY KEY,";
		else if constexpr (schemas::ColType<C>::is_nullable())
			return std::string(schemas::COL_NAME<C>) + " " +
				std::string(schemas::COL_SQL_TYPE<C>) +",";
		else
			return std::string(schemas::COL_NAME<C>) + " " +
				std::string(schemas::COL_SQL_TYPE<C>) + " NOT NULL,";
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
		constexpr int index = static_cast<int>(i) + 1; // sql starts counting params from one
		int status = SQLITE_FAIL;
		if constexpr (NullParam<P>)
			status = sqlite3_bind_null(stmt, index);
		else if constexpr (IntParam<P>)
			status = sqlite3_bind_int64(stmt, index, static_cast<std::int64_t>(value));
		else if constexpr (RealParam<P>)
			status = sqlite3_bind_double(stmt, index, static_cast<double>(value));
		else if constexpr (TextParam<P>)
		{
			const std::string& str = value;
			status = sqlite3_bind_text(stmt, index, 
									   str.c_str(),
									   -1, SQLITE_TRANSIENT);
		}
		else if constexpr (BlobParam<P>)
		{
			const Buffer& bytes = static_cast<const Buffer&>(value);
			status = sqlite3_bind_blob(stmt, index, bytes.data(),
									   static_cast<int>(bytes.size()),
									   SQLITE_STATIC);
		}

		if (SQLITE_OK != status)
			throw SQLiteException("Failed to bind parameter", status);
	}

	template <std::size_t... is, Param... Ps>
	inline void ParamUtils::bind_all(std::index_sequence<is...> dummy, sqlite3_stmt* stmt, const Ps&... values)
	{
		(void)dummy; // for template inference
		(bind_one<is, Ps>(stmt, values), ...);
	}
}

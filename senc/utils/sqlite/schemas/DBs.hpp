/*********************************************************************
 * \file   DBs.hpp
 * \brief  Contains sqlite DB schema utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "../../FixedString.hpp"
#include "tables.hpp"

namespace senc::utils::sqlite::schemas
{
	/**
	 * @struct senc::utils::sqlite::schemas::DB
	 * @brief Schema of database.
	 * @tparam Ts Database tables (schemas).
	 */
	template <SomeTable... Ts>
	requires (!HAS_DUP_TABLES<Ts...>)
	struct DB { };

	namespace sfinae
	{
		// used for detecting any DB schema
		template <typename T>
		struct some_db : std::false_type { };

		template <SomeTable... Ts>
		struct some_db<DB<Ts...>> : std::true_type { };
	}

	/**
	 * @concept senc::utils::sqlite::schemas::SomeDB
	 * @brief Looks for any instantation of a DB schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeDB = sfinae::some_db<Self>::value;

	namespace sfinae
	{
		// used for checking if a DB schema has a table of a specific name
		template <SomeDB D, FixedString tableName>
		struct db_has_table : std::false_type { };

		template <FixedString tableName, SomeTable... Ts>
		struct db_has_table<DB<Ts...>, tableName>
			: std::bool_constant<((tableName == TABLE_NAME<Ts>) || ...)> { };
		// any of `Ts` has name `tableName`
	}

	/**
	 * @concept senc::utils::sqlite::schemas::DBWithTable
	 * @brief Looks for a DB schema which has a table with a specific name.
	 * @tparam Self Examined typename.
	 * @tparam tableName Table name to look for.
	 */
	template <typename Self, FixedString tableName>
	concept DBWithTable = sfinae::db_has_table<Self, tableName>::value;

	namespace sfinae
	{
		// used for retrieving DB table by name
		template <SomeDB D, FixedString tableName>
		requires DBWithTable<D, tableName>
		struct db_table { };

		// if first has name, return. otherwise, continue
		template <FixedString tableName, SomeTable First, SomeTable... Rest>
		struct db_table<DB<First, Rest...>, tableName> : std::conditional<
			(TABLE_NAME<First> == tableName),
			First,
			typename db_table<DB<Rest...>, tableName>::type
		> { };
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::DBTable
	 * @brief Gets table schema from DB schema by name.
	 * @tparam D DB schema.
	 * @tparam tableName Table name.
	 * @note Requires `D` to be `DBWithTable<tableName>`.
	 */
	template <SomeDB D, FixedString tableName>
	requires DBWithTable<D, tableName>
	using DBTable = typename sfinae::db_table<D, tableName>::type;

	namespace sfinae
	{
		// used for checking if a DB schema has a table and column with specific names
		template <SomeDB D, FixedString tableName, FixedString colName>
		struct db_has_table_with_col : std::false_type { };

		template <FixedString tableName, FixedString colName, DBWithTable<tableName> D>
		struct db_has_table_with_col : std::bool_constant<TableWithCol<DBTable<D, tableName>, colName>> { };
	}

	/**
	 * @concept senc::utils::sqlite::schemas::DBWithTableWithCol
	 * @brief Looks for a DB schema which has a table and column with specific names.
	 * @tparam Self Examined typename.
	 * @tparam tableName Table name to look for.
	 * @tparam colName Column name to look for within table.
	 */
	template <typename D, FixedString tableName, FixedString colName>
	concept DBWithTableWithCol = sfinae::db_has_table_with_col<D, tableName, colName>::value;

	namespace sfinae
	{
		// used for retreiving DB table column by names of table and column
		template <SomeDB D, FixedString tableName, FixedString colName>
		requires DBWithTableWithCol<D, tableName, colName>
		struct db_table_col { };

		// if first has table name, invoke table column getter. otherwise, continue
		template <
			FixedString tableName, FixedString colName,
			SomeTable First, SomeTable... Rest
		>
		struct db_table_col<DB<First, Rest...>, tableName, colName> : std::conditional<
			(TABLE_NAME<First> == tableName),
			TableCol<First, colName>,
			typename db_table_col<DB<Rest...>, tableName, colName>::type
		> { };
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::DBTableCol
	 * @brief Gets column schema from DB schema by table & column names.
	 * @tparam D DB schema.
	 * @tparam tableName Table name.
	 * @tparam colName Column name.
	 * @note Requires `D` to be `DBWithTableWithCol<tableName, colName>`.
	 */
	template <SomeDB D, FixedString tableName, FixedString colName>
	requires DBWithTableWithCol<D, tableName, colName>
	using DBTableCol = typename sfinae::db_table_col<D, tableName, colName>::type;
}

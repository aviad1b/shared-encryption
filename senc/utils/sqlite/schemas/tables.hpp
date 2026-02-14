/*********************************************************************
 * \file   tables.hpp
 * \brief  Contains sqlite table schema utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "../../FixedString.hpp"
#include "../args.hpp"
#include "columns.hpp"
#include <tuple>

namespace senc::utils::sqlite::schemas
{
	/**
	 * @struct senc::utils::sqlite::schemas::Table
	 * @brief Schema of table.
	 * @tparam name Table name.
	 * @tparam Cs Table columns (schemas).
	 */
	template <FixedString name, SomeCol... Cs>
	requires (!HAS_DUP_COLS<Cs...>)
	struct Table
	{
		static constexpr FixedString NAME = name;
		using Tuple = std::tuple<ColType<Cs>...>;
	};

	namespace sfinae
	{
		// used for detecting a table schema
		template <typename T>
		struct some_table : std::false_type { };

		template <FixedString name, SomeCol... Cs>
		struct some_table<Table<name, Cs...>> : std::true_type { };
	}

	/**
	 * @concept senc::utils::sqlite::schemas::SomeTable
	 * @brief Looks for any instantation of a table schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeTable = sfinae::some_table<Self>::value;

	/**
	 * @var senc::utils::sqlite::schemas::TABLE_NAME
	 * @brief Gets table name from table schema.
	 * @tparam T Table schema.
	 */
	template <SomeTable T>
	constexpr FixedString TABLE_NAME = T::NAME;

	/**
	 * @typedef senc::utils::sqlite::schemas::TableTuple
	 * @brief Gets tuple version of table schema.
	 * @tparam T Table schema.
	 */
	template <SomeTable T>
	using TableTuple = typename T::Tuple;

	namespace sfinae
	{
		// used for checking if a typename is callable with table column values
		template <typename F, SomeTable T>
		struct is_table_callable : std::false_type { };

		template <typename F, FixedString name, SomeCol... Cs>
		struct is_table_callable<F, Table<name, Cs...>> : std::bool_constant<
			Callable<F, void, ColType<Cs>...>
		>{ };
	}

	/**
	 * @concept senc::utils::sqlite::TableCallable
	 * @brief Looks for a typename that can be called with params taken form a given table schema.
	 * @tparam Self Examined typename.
	 * @tparam T Table schema.
	 */
	template <typename Self, typename T>
	concept TableCallable = sfinae::is_table_callable<Self, T>::value;

	namespace sfinae
	{
		template <SomeTable T, FixedString name>
		struct rename_table { };

		template <FixedString newName, FixedString oldName, SomeCol... Cs>
		struct rename_table<Table<oldName, Cs...>, newName>
		{
			using type = Table<newName, Cs...>;
		};
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::RenameTables
	 * @brief Renames a table schema.
	 * @tparam T Table schema.
	 * @tparam name New name.
	 */
	template <SomeTable T, FixedString name>
	using RenameTable = typename sfinae::rename_table<T, name>::type;

	namespace sfinae
	{
		// used for checking if a table schema has a column of a specific name
		template <SomeTable T, FixedString colName>
		struct table_has_col : std::false_type { };

		template <FixedString colName, FixedString tableName, SomeCol... Cs>
		struct table_has_col<Table<tableName, Cs...>, colName>
			: std::bool_constant<((colName == COL_NAME<Cs>) || ...)> { };
		// any of `Cs` has name `tableName`
	}

	/**
	 * @concept senc::utils::sqlite::schemas::TableWithCol
	 * @brief Looks for a table schema which has a column with a specific name.
	 * @tparam Self Examined typename.
	 * @tparam colName Column name to look for.
	 */
	template <typename Self, FixedString colName>
	concept TableWithCol = sfinae::table_has_col<Self, colName>::value;

	namespace sfinae
	{
		// used for retrieving table column by name
		template <SomeTable T, FixedString colName>
		requires TableWithCol<T, colName>
		struct table_col { };

		// if first has name, return. otherwise, continue
		template <
			FixedString colName, FixedString tableName,
			SomeCol First, SomeCol... Rest
		>
		struct table_col<Table<tableName, First, Rest...>, colName> : std::conditional<
			(COL_NAME<First> == colName),
			First,
			typename table_col<Table<tableName, Rest...>, colName>::type
		> { };
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::TableWithCol
	 * @brief Gets column schema from table schema by name.
	 * @tparam T Table schema.
	 * @tparam colName Column name.
	 * @note Requires `T` to be `TableWithCol<colName>`.
	 */
	template <SomeTable T, FixedString colName>
	requires TableWithCol<T, colName>
	using TableCol = typename sfinae::table_col<T, colName>::type;

	/**
	 * @var senc::utils::sqlite::schemas::IS_DUP_TABLE
	 * @brief Checks if one table schema is duplicate of another (cannot be differentiated).
	 * @tparam T1 First table schema.
	 * @tparam T2 Second table schema.
	 */
	template <SomeTable T1, SomeTable T2>
	constexpr bool IS_DUP_TABLE = (TABLE_NAME<T1> == TABLE_NAME<T2>);

	namespace sfinae
	{
		// used for checking if there are duplicated tables in a pack
		template <SomeTable... Ts>
		struct has_dup_tables : std::false_type { };

		// case zero: empty case, no duplications
		template <>
		struct has_dup_tables<> : std::false_type { };

		// case one: no duplicates (since we only have one table)
		template <SomeTable T>
		struct has_dup_tables<T> : std::false_type { };
		
		// default: for each table, compare to every one that comes after
		template <SomeTable First, SomeTable... Rest>
		struct has_dup_tables<First, Rest...> : std::disjunction<
			std::bool_constant<(IS_DUP_TABLE<First, Rest> || ...)>,
			has_dup_tables<Rest...>
		> { };
	}

	/**
	 * @var senc::utils::sqlite::schemas::HAS_DUP_TABLES
	 * @brief Checks if a pack of table schemas has duplicates (tables can't be differentiated).
	 * @tparam Ts Table schemas.
	 */
	template <SomeTable... Ts>
	constexpr bool HAS_DUP_TABLES = sfinae::has_dup_tables<Ts...>::value;

	namespace sfinae
	{
		template <SomeTable T, typename... Ps>
		struct params_for_table : std::false_type { };

		template <FixedString name, SomeCol... Cs, typename... Ps>
		struct params_for_table<Table<name, Cs...>, Ps...> : std::bool_constant<
			(ParamOfValue<Ps, ColType<Cs>> && ...)
		> { };
	}

	/**
	 * @var senc::utils::sqlite::schemas::PARAMS_FOR_TABLE
	 * @brief Checks if given typename params are fitting for insertion into a table.
	 * @tparam T Table schema.
	 * @tparam Ps Param typenames.
	 */
	template <SomeTable T, typename... Ps>
	constexpr bool PARAMS_FOR_TABLE = sfinae::params_for_table<T, Ps...>::value;

	namespace sfinae
	{
		// used for converting table schema to a select query
		template <SomeTable T>
		struct table_to_select { };

		template <FixedString name, SomeCol C>
		struct table_to_select<Table<name, C>> : FixedStringConstant<
			"SELECT " + COL_FULL_NAME<C> + " FROM " + TABLE_NAME<Table<name, C>>
		> { };

		template <FixedString name, SomeCol First, SomeCol... Rest>
		struct table_to_select<Table<name, First, Rest...>> : FixedStringConstant<
			"SELECT " + COL_FULL_NAME<First> + ((", " + COL_FULL_NAME<Rest>) + ...) +
			" FROM " + TABLE_NAME<Table<name, First, Rest...>>
		> { };
	}

	/**
	 * @var senc::utils::sqlite::schemas::TableToSelect
	 * @brief Converts table schema to a matching select query.
	 * @tparam T Table schema.
	 */
	template <SomeTable T>
	constexpr FixedString TABLE_TO_SELECT = sfinae::table_to_select<T>::value;

	namespace sfinae
	{
		// utility sfinae used for concating a column at a table's head
		template <SomeTable T, SomeCol HC>
		struct concat_table_head { };

		template <SomeCol HC, FixedString tableName, SomeCol... Cs>
		struct concat_table_head<Table<tableName, Cs...>, HC>
		{
			using type = Table<tableName, HC, Cs...>;
		};


		// utility sfinae used for concating a column at a table's tail
		template <SomeTable T, SomeCol TC>
		struct concat_table_tail { };

		template <SomeCol TC, FixedString tableName, SomeCol... Cs>
		struct concat_table_tail<Table<tableName, Cs...>, TC>
		{
			using type = Table<tableName, Cs..., TC>;
		};


		// utility sfinae used for concating columns at a table's tail
		template <SomeTable T, SomeCol... TCs>
		struct concat_table_tails { };

		// case zero: nothing to concat
		template <SomeTable T>
		struct concat_table_tails<T> { using type = T; };

		// default: concat first, then the rest
		template <SomeTable T, SomeCol First, SomeCol... Rest>
		struct concat_table_tails<T, First, Rest...> : concat_table_tails<
			typename concat_table_tail<T, First>::type,
			Rest...
		> { };


		// utility sfinae used for concating two tables
		template <SomeTable T1, SomeTable T2, FixedString name = TABLE_NAME<T1>>
		struct concat_tables { };

		template <FixedString name, SomeTable T1, FixedString name2, SomeCol... Cs2>
		struct concat_tables<T1, Table<name2, Cs2...>, name> : concat_table_tails<
			RenameTable<T1, name>,
			Cs2...
		> { };


		// utility sfinae used for removing owner from col found by name
		template <SomeTable T, FixedString colName>
		struct remove_table_col_owner { };

		// case zero: no columns, nothing to remove
		template <FixedString tableName, FixedString colName>
		struct remove_table_col_owner<Table<tableName>, colName>
		{
			using type = Table<tableName>;
		};

		// default: if first has name, remove its owner and keep rest as is.
		//          otherwise, keep first as is and run through rest
		template <
			FixedString tableName, FixedString colName,
			SomeCol First, SomeCol... Rest
		>
		struct remove_table_col_owner<Table<tableName, First, Rest...>, colName> : std::conditional<
			(COL_NAME<First> == colName),
			typename concat_table_head<
				Table<tableName, Rest...>,
				RemoveColOwner<First>
			>::type,
			typename concat_table_head<
				typename remove_table_col_owner<Table<tableName, Rest...>, colName>::type,
				First
			>::type
		> { };


		// utility sfinae used for dropping col found by name
		template <SomeTable T, FixedString colName>
		struct drop_table_col_by_name { };

		// case zero: no columns, nothing to remove
		template <FixedString tableName, FixedString colName>
		struct drop_table_col_by_name<Table<tableName>, colName>
		{
			using type = Table<tableName>;
		};

		// default: if first has name, drop it and keep rest as is.
		//          otherwise, keep first as is and run through rest
		template <
			FixedString tableName, FixedString colName,
			SomeCol First, SomeCol... Rest
		>
		struct drop_table_col_by_name<Table<tableName, First, Rest...>, colName> : std::conditional<
			(COL_NAME<First> == colName),
			Table<tableName, Rest...>,
			typename concat_table_head<
				typename drop_table_col_by_name<Table<tableName, Rest...>, colName>::type,
				First
			>::type
		> { };
	}
}

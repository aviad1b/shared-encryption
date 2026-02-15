/*********************************************************************
 * \file   select.hpp
 * \brief  Contains sqlite select schema utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "../../FixedString.hpp"
#include "../args.hpp"
#include "tables.hpp"

namespace senc::utils::sqlite::schemas
{
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
		template <SomeCol C, SomeSelectArg Arg>
		struct col_matches_select_arg : std::conjunction<
			std::disjunction<
				std::negation<some_owned_col<C>>,
				std::negation<sqlite::sfinae::some_select_arg_with_owner<Arg>>,
				std::conjunction<
					some_owned_col<C>,
					sqlite::sfinae::some_select_arg_with_owner<Arg>,
					utils::sfinae::is_same_fixed_string<COL_OWNER<C>, SELECT_ARG_OWNER<Arg>>
				>
			>, // if both have owner, should be same; otherwise, any owner is okay
			utils::sfinae::is_same_fixed_string<COL_NAME<C>, SELECT_ARG_UNDERLYING_NAME<Arg>> // in addition, check same name
		> { };
	}

	/**
	 * @var senc::utils::Sqlite::schemas::COL_MATCHES_SELECT_ARG
	 * @brief Checks if a given column schema should be captured by a given select argument.
	 * @tparam C Column schema.
	 * @tparam Arg Select argument.
	 */
	template <SomeCol C, SomeSelectArg Arg>
	constexpr bool COL_MATCHES_SELECT_ARG = sfinae::col_matches_select_arg<C, Arg>::value;

	namespace sfinae
	{
		// used to check if a table has a column matching a given select arg
		template <SomeTable T, SomeSelectArg Arg>
		struct table_has_select_col : std::false_type { };

		template <SomeSelectArg Arg, FixedString name, SomeCol First, SomeCol... Rest>
		struct table_has_select_col<Table<name, First, Rest...>, Arg> : std::disjunction<
			col_matches_select_arg<First, Arg>,
			table_has_select_col<Table<name, Rest...>, Arg>
		> { }; // if `First`'s name matches `Arg`'s, apply inner condition. else - recursion
	}

	/**
	 * @concept senc::utils::sqlite::schemas::TableWithSelectCol
	 * @brief Looks for a table schema which has a column matching a given select arg.
	 * @tparam Self Examined typename.
	 * @tparam Arg Select arg.
	 */
	template <typename Self, typename Arg>
	concept TableWithSelectCol = SomeTable<Self> && SomeSelectArg<Arg> &&
		sfinae::table_has_select_col<Self, Arg>::value;

	/**
	 * @concept senc::utils::sqlite::schemas::Selectable
	 * @brief Looks for a table schema on which given select arguments can be applied.
	 * @tparam Self Examined typename.
	 * @tparam Args Select arguments.
	 */
	template <typename Self, typename... Args>
	concept Selectable = SomeTable<Self> &&                 // `Self` is `Table`
		(SomeSelectArg<Args> && ...) &&                     // Each of `Args` is `SelectArg`
		(TableWithSelectCol<Self, Args> && ...); // `Self` has name of each of `Args`

	namespace sfinae
	{
		// used for applying a single select arg on a table schema
		template <SomeTable T, SomeSelectArg Arg>
		struct select_one { using type = void; };

		template <SomeSelectArg Arg, FixedString name, SomeCol First, SomeCol... Rest>
		struct select_one<Table<name, First, Rest...>, Arg> : std::conditional<
			COL_MATCHES_SELECT_ARG<First, Arg>,
			std::conditional_t<
				SomeSelectArgWithAs<Arg>,
				RenameCol<First, SELECT_ARG_AS<Arg>>,
				First
			>, // if `Arg` has "as", rename `First` accordingly
			typename select_one<Table<name, Rest...>, Arg>::type
		> { }; // if `First`'s name matches `Arg`'s, apply inner condition. else - recursion

		// used for applying multiple select args on a table schema
		template <SomeTable T, SomeSelectArg... Args>
		struct select
		{
			using type = Table<TABLE_NAME<T>, typename select_one<T, Args>::type...>;
		};
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::Select
	 * @brief Schema return type of a select invocation on a table schema.
	 * @tparam T Table schema.
	 * @tparam Args Select arguments.
	 */
	template <SomeTable T, SomeSelectArg... Args>
	requires Selectable<T, Args...>
	using Select = typename sfinae::select<T, Args...>::type;
}

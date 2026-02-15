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
		// used for checking if a source column matches a select arg
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

		// used for checking if a destination column matches a select arg
		template <SomeCol C, SomeSelectArg Arg>
		struct res_col_matches_select_arg : std::disjunction<
			// either arg has "as" and column fits this "as",
			// or arg does not have "as" and column fits as a source column
			std::conjunction<
				sqlite::sfinae::some_select_arg_with_as<Arg>,
				utils::sfinae::is_same_fixed_string<SELECT_ARG_AS<Arg>, COL_NAME<C>>
			>,
			std::conjunction<
				std::negation<sqlite::sfinae::some_select_arg_with_as<Arg>>,
				col_matches_select_arg<C, Arg>
			>
		> { };
	}

	/**
	 * @var senc::utils::sqlite::schemas::COL_MATCHES_SELECT_ARG
	 * @brief Checks if a given column schema should be captured by a given select argument.
	 * @tparam C Column schema.
	 * @tparam Arg Select argument.
	 */
	template <SomeCol C, SomeSelectArg Arg>
	constexpr bool COL_MATCHES_SELECT_ARG = sfinae::col_matches_select_arg<C, Arg>::value;

	/**
	 * @var senc::utils::sqlite::schemas::RES_COL_MATCHES_SELECT_ARG
	 * @brief Checks if a given select result column schema should be captured by a given select argument.
	 * @tparam C Column schema.
	 * @tparam Arg Select argument.
	 */
	template <SomeCol C, SomeSelectArg Arg>
	constexpr bool RES_COL_MATCHES_SELECT_ARG = sfinae::res_col_matches_select_arg<C, Arg>::value;

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

	namespace sfinae
	{
		// used for getting select arg that's fitting for a column of the result schema
		template <SomeCol C, SomeSelectArgsCollection Args>
		struct select_arg_of_res_col { using type = void; };

		template <SomeCol C, SomeSelectArg First, SomeSelectArg... Rest>
		struct select_arg_of_res_col<C, SelectArgsCollection<First, Rest...>> : std::conditional<
			RES_COL_MATCHES_SELECT_ARG<C, First>,
			First,
			typename select_arg_of_res_col<C, SelectArgsCollection<Rest...>>::type
		> { };
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::SelectArgOfResCol
	 * @brief Gets select arg that's fitting for a column of the result schema.
	 * @tparam C Column schema.
	 * @tparam Args Select args (collection).
	 */
	template <SomeCol C, SomeSelectArgsCollection Args>
	using SelectArgOfResCol = typename sfinae::select_arg_of_res_col<C, Args>::type;

	namespace sfinae
	{
		// used to get column name as used in select
		template <SomeCol C, SomeSelectArgsCollection Args,
			bool renamed = SomeSelectArgWithAs<SelectArgOfResCol<C, Args>>>
		requires SomeSelectArg<SelectArgOfResCol<C, Args>>
		struct col_select_name : col_full_name<C> { };

		template <SomeCol C, SomeSelectArgsCollection Args>
		requires SomeSelectArgWithAs<SelectArgOfResCol<C, Args>>
		struct col_select_name<C, Args, true> : FixedStringConstant<
			COL_FULL_NAME<C> + " AS " + SELECT_ARG_AS<SelectArgOfResCol<C, Args>>
		> { };
	}

	/**
	 * @var senc::utils::sqlite::schemas::COL_SELECT_NAME
	 * @brief Gets column name as used in select.
	 * @tparam C Column schema.
	 * @tparam Args Select args (collection).
	 */
	template <SomeCol C, SomeSelectArgsCollection Args>
	constexpr FixedString COL_SELECT_NAME = sfinae::col_select_name<C, Args>::value;

	namespace sfinae
	{
		// used for converting table schema to a select query
		template <SomeTable T, SomeSelectArgsCollection Args>
		struct table_to_select { };

		template <SomeSelectArgsCollection Args, FixedString name, SomeCol C>
		struct table_to_select<Table<name, C>, Args> : FixedStringConstant<
			"SELECT " + COL_SELECT_NAME<C, Args> + " FROM " + TABLE_NAME<Table<name, C>>
		> { };

		template <SomeSelectArgsCollection Args, FixedString name, SomeCol First, SomeCol... Rest>
		struct table_to_select<Table<name, First, Rest...>, Args> : FixedStringConstant<
			"SELECT " + COL_SELECT_NAME<First, Args> + ((", " + COL_FULL_NAME<Rest>) + ...) +
			" FROM " + TABLE_NAME<Table<name, First, Rest...>>
		> { };
	}

	/**
	 * @var senc::utils::sqlite::schemas::TableToSelect
	 * @brief Converts table schema to a matching select query.
	 * @tparam T Table schema.
	 * @tparam Args Select args (collection).
	 */
	template <SomeTable T, SomeSelectArgsCollection Args>
	constexpr FixedString TABLE_TO_SELECT = sfinae::table_to_select<T, Args>::value;
}

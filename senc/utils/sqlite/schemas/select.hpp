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
	/**
	 * @var senc::utils::Sqlite::schemas::COL_MATCHES_SELECT_ARG
	 * @brief Checks if a given column schema should be captured by a given select argument.
	 * @tparam C Column schema.
	 * @tparam Arg Select argument.
	 */
	template <SomeCol C, SomeSelectArg Arg>
	constexpr bool COL_MATCHES_SELECT_ARG =
		(
			!SomeOwnedCol<C> ||
			!SomeSelectArgWithOwner<Arg> ||
			(SomeOwnedCol<C> && SomeSelectArgWithOwner<Arg> &&
				COL_OWNER<C> == SELECT_ARG_OWNER<Arg>)
		) && // if both have owner, should be same; otherwise, any owner is okay
		(COL_NAME<C> == SELECT_ARG_NAME<Arg>); // in addition, check same name

	/**
	 * @concept senc::utils::sqlite::schemas::Selectable
	 * @brief Looks for a table schema on which given select arguments can be applied.
	 * @tparam Self Examined typename.
	 * @tparam Args Select arguments.
	 */
	template <typename Self, typename... Args>
	concept Selectable = SomeTable<Self> &&                 // `Self` is `Table`
		(SomeSelectArg<Args> && ...) &&                     // Each of `Args` is `SelectArg`
		(TableWithCol<Self, SELECT_ARG_NAME<Args>> && ...); // `Self` has name of each of `Args`

	namespace sfinae
	{
		// used for applying a single select arg on a table schema
		template <SomeTable T, SomeSelectArg Arg>
		struct select_one { };

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

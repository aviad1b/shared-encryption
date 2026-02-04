/*********************************************************************
 * \file   join.hpp
 * \brief  Contains sqlite join schema utilities.
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
	 * @concept senc::utils::sqlite::schemas::Joinable
	 * @brief Checks if two tables are joinable (by schemas) on a given column name.
	 * @tparam Self First table schema.
	 * @tparam Other Second table schema.
	 * @tparam name Column name.
	 */
	template <typename Self, typename Other, FixedString name>
	concept Joinable = TableWithCol<Self, name> && TableWithCol<Other, name>;

	namespace sfinae
	{
		// used for joining two tables on a named col
		template <SomeTable T1, SomeTable T2, FixedString name>
		struct join : concat_tables<
			typename remove_table_col_owner<T1, name>::type,
			typename drop_table_col_by_name<T2, name>::type,
			TABLE_NAME<T1> + " JOIN " + TABLE_NAME<T2> + " ON " + name
		> { };
		// for axis col: remove owner from first table and drop from second.
		// then, concat the two results
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::Join
	 * @brief Schema return type of a join invocation on two table schemas with a column name.
	 * @tparam T1 First table schema.
	 * @tparam T2 Second table schema.
	 * @tparam name Column name.
	 */
	template <SomeTable T1, SomeTable T2, FixedString name>
	requires Joinable<T1, T2, name>
	using Join = typename sfinae::join<T1, T2, name>::type;
}

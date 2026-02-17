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
	 * @tparam selfColName Name of column in `Self` to join on.
	 * @tparam Other Second table schema.
	 * @tparam otherColName Name of column in `Other` to join on (defaults to `selfColName`).
	 */
	template <typename Self, FixedString selfColName, typename Other, FixedString otherColName = selfColName>
	concept Joinable = TableWithCol<Self, selfColName> && TableWithCol<Other, otherColName>;

	namespace sfinae
	{
		// used for joining two tables on a named col
		template <SomeTable T1, FixedString colName1, SomeTable T2, FixedString colName2 = colName1>
		struct join : concat_tables<
			RemoveTableColOwner<T1, colName1>,
			DropTableColByName<T2, colName2>,
			TABLE_NAME<T1> + " JOIN " + TABLE_NAME<T2> + " ON " + 
			COL_FULL_NAME<TableCol<T1, colName1>> + " = " + 
			COL_FULL_NAME<TableCol<T2, colName2>>
		> { };
		// for axis col: remove owner from first table and drop from second.
		// then, concat the two results
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::Join
	 * @brief Schema return type of a join invocation on two table schemas with a column name.
	 * @tparam T1 First table schema.
	 * @tparam colName1 Name of column in `T1` to join on.
	 * @tparam T2 Second table schema.
	 * @tparam colName2 Name of column in `T2` to join on.
	 */
	template <SomeTable T1, FixedString colName1, SomeTable T2, FixedString colName2>
	requires Joinable<T1, colName1, T2, colName2>
	using Join = typename sfinae::join<T1, colName1, T2, colName2>::type;
}

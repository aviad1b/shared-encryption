/*********************************************************************
 * \file   columns.hpp
 * \brief  Contains sqlite column schema utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "../../FixedString.hpp"
#include "../values.hpp"

namespace senc::utils::sqlite::schemas
{
	/**
	 * @struct senc::utils::sqlite::schemas::Col
	 * @brief Schema of table column.
	 * @tparam name Column name.
	 * @tparam V Column value type.
	 */
	template <FixedString name, Value V>
	struct Col
	{
		static constexpr FixedString NAME = name;
		using Type = V;
	};

	/**
	 * @struct senc::utils::sqlite::schemas::OwnedCol
	 * @brief Schema of table column, which knows the table's name.
	 * @tparam owner Table name.
	 * @tparam name Column name.
	 * @tparam V Column value type.
	 */
	template <FixedString owner, FixedString name, Value V>
	struct OwnedCol
	{
		static constexpr FixedString OWNER = owner;
		static constexpr FixedString NAME = name;
		using Type = V;
	};

	/**
	 * @struct senc::utils::sqlite::schemas::PrimaryKey
	 * @brief Schema for table column which serves as a primary key.
	 * @tparam name Column name.
	 * @tparam V Column value type.
	 */
	template <FixedString name, Value V>
	struct PrimaryKey
	{
		static constexpr FixedString NAME = name;
		using Type = V;
	};

	/**
	 * @struct senc::utils::sqlite::schemas::OwnedPrimaryKey
	 * @brief Schema for table column which serves as a primary key, and knows the table name.
	 * @tparam owner Table name.
	 * @tparam name Column name.
	 * @tparam V Column value type.
	 */
	template <FixedString owner, FixedString name, Value V>
	struct OwnedPrimaryKey
	{
		static constexpr FixedString OWNER = owner;
		static constexpr FixedString NAME = name;
		using Type = V;
	};

	/**
	 * @struct senc::utils::sqlite::schemas::ForeignKey
	 * @brief Schema for table column which serves as a foreign key.
	 * @tparam name Column name.
	 * @tparam V Column value type.
	 * @tparam refTable Name of referenced table.
	 * @tparam refCol Name of reference column.
	 */
	template <FixedString name, Value V, FixedString refTable, FixedString refCol>
	struct ForeignKey
	{
		static constexpr FixedString NAME = name;
		using Type = V;
		static constexpr FixedString REF_TABLE_NAME = refTable;
		static constexpr FixedString REF_COL_NAME = refCol;
	};

	/**
	 * @struct senc::utils::sqlite::schemas::OwnedForeignKey
	 * @brief Schema for table column which serves as a foreign key, and knows the table name.
	 * @tparam owner Table name.
	 * @tparam name Column name.
	 * @tparam V Column value type.
	 * @tparam refTable Name of referenced table.
	 * @tparam refCol Name of reference column.
	 */
	template <FixedString owner, FixedString name, Value V, FixedString refTable, FixedString refCol>
	struct OwnedForeignKey
	{
		static constexpr FixedString OWNER = owner;
		static constexpr FixedString NAME = name;
		using Type = V;
		static constexpr FixedString REF_TABLE_NAME = refTable;
		static constexpr FixedString REF_COL_NAME = refCol;
	};

	namespace sfinae
	{
		// used for detecting a column schema (whether knows table or not)
		template <typename T>
		struct some_col : std::false_type { };

		template <FixedString name, Value V>
		struct some_col<Col<name, V>> : std::true_type { };

		template <FixedString owner, FixedString name, Value V>
		struct some_col<OwnedCol<owner, name, V>> : std::true_type { };

		template <FixedString name, Value V>
		struct some_col<PrimaryKey<name, V>> : std::true_type { };

		template <FixedString owner, FixedString name, Value V>
		struct some_col<OwnedPrimaryKey<owner, name, V>> : std::true_type { };

		template <FixedString name, Value V, FixedString refTable, FixedString refCol>
		struct some_col<ForeignKey<name, V, refTable, refCol>> : std::true_type { };

		template <FixedString owner, FixedString name, Value V, FixedString refTable, FixedString refCol>
		struct some_col<OwnedForeignKey<owner, name, V, refTable, refCol>> : std::true_type { };

		// used for detecting a column schema which knows table name
		template <typename T>
		struct some_owned_col : std::false_type { };

		template <FixedString owner, FixedString name, Value V>
		struct some_owned_col<OwnedCol<owner, name, V>> : std::true_type { };

		template <FixedString owner, FixedString name, Value V>
		struct some_owned_col<OwnedPrimaryKey<owner, name, V>> : std::true_type { };

		template <FixedString owner, FixedString name, Value V, FixedString refTable, FixedString refCol>
		struct some_owned_col<OwnedForeignKey<owner, name, V, refTable, refCol>> : std::true_type { };

		// used for detecting a primary key column schema
		template <typename T>
		struct some_primary_key : std::false_type { };

		template <FixedString name, Value V>
		struct some_primary_key<PrimaryKey<name, V>> : std::true_type { };

		template <FixedString owner, FixedString name, Value V>
		struct some_primary_key<OwnedPrimaryKey<owner, name, V>> : std::true_type { };

		// used for detecting a primary key column schema which knows table name
		template <typename T>
		struct some_owned_primary_key : std::false_type { };

		template <FixedString owner, FixedString name, Value V>
		struct some_owned_primary_key<OwnedPrimaryKey<owner, name, V>> : std::true_type { };

		// used for detecting a foreign key column schema
		template <typename T>
		struct some_foreign_key : std::false_type { };

		template <FixedString name, Value V, FixedString refTable, FixedString refCol>
		struct some_foreign_key<ForeignKey<name, V, refTable, refCol>> : std::true_type { };

		template <FixedString owner, FixedString name, Value V, FixedString refTable, FixedString refCol>
		struct some_foreign_key<OwnedForeignKey<owner, name, V, refTable, refCol>> : std::true_type { };

		// used for detecting a foreign key column schema which knows table name
		template <typename T>
		struct some_owned_foreign_key : std::false_type { };

		template <FixedString owner, FixedString name, Value V, FixedString refTable, FixedString refCol>
		struct some_owned_foreign_key<OwnedForeignKey<owner, name, V, refTable, refCol>> : std::true_type { };
	}

	/**
	 * @concept senc::utils::sqlite::schemas::SomeCol
	 * @brief Looks for any instantation of a column schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeCol = sfinae::some_col<Self>::value;

	/**
	 * @concept senc::utils::sqlite::schemas::SomeOwnedCol
	 * @brief Looks for any instantation of a table-knowing column schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeOwnedCol = sfinae::some_owned_col<Self>::value;

	/**
	 * @concept senc::utils::sqlite::schemas::SomePrimaryKey
	 * @brief Looks for any instantation of a primary key column schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomePrimaryKey = sfinae::some_primary_key<Self>::value;

	/**
	 * @concept senc::utils::sqlite::schemas::SomeOwnedPrimaryKey
	 * @brief Looks for any instantation of a table-knowing primary key column schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeOwnedPrimaryKey = sfinae::some_owned_primary_key<Self>::value;

	/**
	 * @concept senc::utils::sqlite::schemas::SomeForeignKey
	 * @brief Looks for any instantation of a foreign key column schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeForeignKey = sfinae::some_foreign_key<Self>::value;

	/**
	 * @concept senc::utils::sqlite::schemas::SomeOwnedForeignKey
	 * @brief Looks for any instantation of a table-knowing foreign key column schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeOwnedForeignKey = sfinae::some_owned_foreign_key<Self>::value;

	namespace sfinae
	{
		// used for getting the table name from a column schema which knows it
		template <SomeCol C, bool owned = SomeOwnedCol<C>>
		struct col_owner : EmptyFixedStringConstant { };

		template <SomeOwnedCol C>
		struct col_owner<C, true> : FixedStringConstant<C::OWNER> { };

		// used for getting a column name from column schema
		template <SomeCol C>
		struct col_name : FixedStringConstant<C::NAME> { };

		// used for getting full name of column (including table name if it knows it)
		template <SomeCol C, bool owned = SomeOwnedCol<C>>
		struct col_full_name : col_name<C> { };

		template <SomeOwnedCol C>
		struct col_full_name<C, true> : FixedStringConstant<
			col_owner<C>::value + "." + col_name<C>::value
		> { };

		// used for getting the referenced table name from a foreign key column schema
		template <SomeCol C, bool foreignKey = SomeForeignKey<C>>
		struct foreign_key_ref_table_name : EmptyFixedStringConstant { };

		template <SomeForeignKey C>
		struct foreign_key_ref_table_name<C, true> : FixedStringConstant<C::REF_TABLE_NAME> { };

		// used for getting the referenced column name from a foreign key column schema
		template <SomeCol C, bool foreignKey = SomeForeignKey<C>>
		struct foreign_key_ref_col_name : EmptyFixedStringConstant { };

		template <SomeForeignKey C>
		struct foreign_key_ref_col_name<C, true> : FixedStringConstant<C::REF_COL_NAME> { };

		// used for getting column value type form column schema.
		template <SomeCol C>
		struct col_type { using type = typename C::Type; };
	}

	/**
	 * @var senc::utils::sqlite::schemas::COL_OWNER
	 * @brief Gets table name from column schema which knows it (or empty if it doesn't).
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	constexpr FixedString COL_OWNER = sfinae::col_owner<C>::value;

	/**
	 * @var senc::utils::sqlite::schemas::COL_NAME
	 * @brief Gets column name from column schema.
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	constexpr FixedString COL_NAME = sfinae::col_name<C>::value;

	/**
	 * @var senc::utils::sqlite::schemas::COL_FULL_NAME
	 * @brief Gets full name of column schema (including table name, if knows it).
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	constexpr FixedString COL_FULL_NAME = sfinae::col_full_name<C>::value;

	/**
	 * @var senc::utils::sqlite::schemas::FOREIGN_KEY_REF_TABLE_NAME
	 * @brief Gets referenced table name from a foreign key column schema (empty if not foreign key).
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	constexpr FixedString FOREIGN_KEY_REF_TABLE_NAME = sfinae::foreign_key_ref_table_name<C>::value;

	/**
	 * @var senc::utils::sqlite::schemas::FOREIGN_KEY_REF_COL_NAME
	 * @brief Gets referenced column name from a foreign key column schema (empty if not foreign key).
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	constexpr FixedString FOREIGN_KEY_REF_COL_NAME = sfinae::foreign_key_ref_col_name<C>::value;

	/**
	 * @typedef senc::utils::sqlite::schemas::ColType
	 * @brief Gets column value type form column schema.
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	using ColType = typename sfinae::col_type<C>::type;

	/**
	 * @typedef senc::utils::sqlite::schemas::ColView
	 * @brief Gets column value view from column schema.
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	using ColView = typename ColType<C>::View;

	/**
	 * @var senc::utils::sqlite::schemas::COL_SQL_TYPE
	 * @brief Gets column SQL type from column schema.
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	constexpr FixedString COL_SQL_TYPE = ColType<C>::SQL_TYPE;

	/**
	 * @typedef senc::utils::sqlite::schemas::RenameCol
	 * @brief Gets a renamed version of a column schema.
	 * @tparam C Column schema.
	 * @tparam name New name for column.
	 */
	template <SomeCol C, FixedString name>
	using RenameCol = Col<name, ColType<C>>;

	/**
	 * @typedef senc::utils::sqlite::schemas::RemoveColOwner
	 * @brief Gets a variation of a given column schema which does not remember the table name.
	 * @tparam C Column schema.
	 */
	template <SomeCol C>
	using RemoveColOwner = Col<
		COL_NAME<C>,
		ColType<C>
	>;

	namespace sfinae
	{
		// used for setting table remembered by column schema
		template <SomeCol C, FixedString owner>
		struct set_col_owner : std::conditional<
			SomePrimaryKey<C>,
			OwnedPrimaryKey<owner, COL_NAME<C>, ColType<C>>,
			std::conditional_t<
				SomeForeignKey<C>,
				OwnedForeignKey<
					owner,
					COL_NAME<C>,
					ColType<C>,
					FOREIGN_KEY_REF_TABLE_NAME<C>,
					FOREIGN_KEY_REF_COL_NAME<C>
				>,
				OwnedCol<owner, COL_NAME<C>, ColType<C>>
			>
		> { };
	}

	/**
	 * @typedef senc::utils::sqlite::schemas::SetColOwner
	 * @brief Sets table name remembered by column schema.
	 * @tparam C Column schema.
	 * @tparam owner Table name to make column schema remember.
	 */
	template <SomeCol C, FixedString owner>
	using SetColOwner = typename sfinae::set_col_owner<C, owner>::type;

	/**
	 * @var senc::utils::sqlite::schemas::IS_DUP_COL
	 * @brief Checks if a column schema is duplicate of another (same name with no way of differentiating).
	 * @tparam C1 First column schema.
	 * @tparam C2 Second column schema.
	 */
	template <SomeCol C1, SomeCol C2>
	constexpr bool IS_DUP_COL =
		(COL_NAME<C1> == COL_NAME<C2>) &&
		(
			!SomeOwnedCol<C1> ||
			!SomeOwnedCol<C2> ||
			(SomeOwnedCol<C1> && SomeOwnedCol<C2> &&
				COL_OWNER<C1> == COL_OWNER<C2>)
		);
	// considered duplicated when has same name and has no owner protection
	// (either one has no owner, or both have same owner)

	namespace sfinae
	{
		// used for checking if there are duplicated cols in a pack
		template <SomeCol... Cs>
		struct has_dup_cols : std::false_type { };

		// case zero: empty case, no duplications
		template <>
		struct has_dup_cols<> : std::false_type { };

		// case one: no duplicates (since we only have one col)
		template <SomeCol C>
		struct has_dup_cols<C> : std::false_type { };
		
		// default: for each col, compare to every one that comes after
		template <SomeCol First, SomeCol... Rest>
		struct has_dup_cols<First, Rest...> : std::disjunction<
			std::bool_constant<(IS_DUP_COL<First, Rest> || ...)>,
			has_dup_cols<Rest...>
		> { };
	}

	/**
	 * @var senc::utils::sqlite::schemas::HAS_DUP_COLS
	 * @brief Checks if a pack of column schemas has duplicates (columns can't be differentiated).
	 * @tparam Cs Column schemas.
	 */
	template <SomeCol... Cs>
	constexpr bool HAS_DUP_COLS = sfinae::has_dup_cols<Cs...>::value;
}

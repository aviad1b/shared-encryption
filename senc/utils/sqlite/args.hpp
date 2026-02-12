/*********************************************************************
 * \file   args.hpp
 * \brief  Contains argument types for sqlite utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "../FixedString.hpp"
#include "aggr.hpp"

namespace senc::utils::sqlite
{
	/**
	 * @concept senc::utils::sqlite::NullParam
	 * @brief Looks for a typenam that can be used as a statement null parameter.
	 */
	template <typename Self>
	concept NullParam = OneOf<Self, std::nullptr_t, std::nullopt_t>;

	/**
	 * @concept senc::utils::sqlite::IntParam
	 * @brief Looks for a typenam that can be used as a statement int parameter.
	 */
	template <typename Self>
	concept IntParam = std::convertible_to<Self, std::int64_t> && !std::floating_point<Self>;

	/**
	 * @concept senc::utils::sqlite::RealParam
	 * @brief Looks for a typenam that can be used as a statement real parameter.
	 */
	template <typename Self>
	concept RealParam = std::convertible_to<Self, double> && !std::integral<Self>;

	/**
	 * @concept senc::utils::sqlite::TextParam
	 * @brief Looks for a typenam that can be used as a statement text parameter.
	 */
	template <typename Self>
	concept TextParam = std::convertible_to<Self, std::string>;

	/**
	 * @concept senc::utils::sqlite::BlobParam
	 * @brief Looks for a typenam that can be used as a statement blob parameter.
	 */
	template <typename Self>
	concept BlobParam = std::convertible_to<Self, Buffer>;

	/**
	 * @concept senc::utils::sqlite::Param
	 * @brief Looks for a typenam that can be used as a statement parameter.
	 */
	template <typename Self>
	concept Param = NullParam<Self> || IntParam<Self> || RealParam<Self> || TextParam<Self> || BlobParam<Self>;

	/**
	 * @struct senc::utils::sqlite::SelectArg
	 * @brief Template-level select query argument.
	 * @tparam name Name of column to select.
	 * @tparam as Optional name to rename column to in result schema.
	 */
	template <FixedString name, FixedString as = "">
	struct SelectArg
	{
		static constexpr FixedString NAME = name;
		static constexpr FixedString AS = as;
	};

	/**
	 * @struct senc::utils::sqlite::SelectArgWithOwner
	 * @brief Template-level select query argument.
	 * @tparam owner Containing table name.
	 * @tparam name Name of column to select.
	 * @tparam as Optional name to rename column to in result schema.
	 */
	template <FixedString owner, FixedString name, FixedString as = "">
	struct SelectArgWithOwner
	{
		static constexpr FixedString OWNER = owner;
		static constexpr FixedString NAME = name;
		static constexpr FixedString AS = as;
	};

	/**
	 * @struct senc::utils::sqlite::AggrSelectArg
	 * @brief Template-level select query argument that uses an aggregate function.
	 * @tparam F Aggregate function.
	 * @tparam as Optional name to rename result to in result schema.
	 */
	template <AggrFunc F, FixedString as = "">
	struct AggrSelectArg
	{
		using Func = F;
		static constexpr FixedString NAME =
			AGGR_FUNC_NAME<F> + "(" + AGGR_FUNC_COL<F> + ")";
		static constexpr FixedString AS = as;
	};

	template <AggrFuncWithOwner F, FixedString as>
	struct AggrSelectArg<F, as>
	{
		using Func = F;
		static constexpr FixedString NAME =
			AGGR_FUNC_NAME<F> +"(" + AGGR_FUNC_OWNER<F> + "." + AGGR_FUNC_COL<F> + ")";
		static constexpr FixedString AS = as;
	};

	namespace sfinae
	{
		// used for detecting a select argument
		template <typename T>
		struct some_select_arg : std::false_type { };

		template <FixedString name, FixedString as>
		struct some_select_arg<SelectArg<name, as>> : std::true_type { };

		template <FixedString owner, FixedString name, FixedString as>
		struct some_select_arg<SelectArgWithOwner<owner, name, as>> : std::true_type { };

		template <AggrFunc F, FixedString as>
		struct some_select_arg<AggrSelectArg<F, as>> : std::true_type { };

		// used for detecting a select argument with a containing table name
		template <typename T>
		struct some_select_arg_with_owner : std::false_type { };

		template <FixedString owner, FixedString name, FixedString as>
		struct some_select_arg_with_owner<SelectArgWithOwner<owner, name, as>> : std::true_type { };

		// used for detecting a select argument with a rename column name
		template <typename T>
		struct some_select_arg_with_as : std::false_type { };

		template <FixedString name, FixedString as>
		struct some_select_arg_with_as<SelectArg<name, as>>
			: std::bool_constant<!as.empty()> { };

		template <FixedString owner, FixedString name, FixedString as>
		struct some_select_arg_with_as<SelectArgWithOwner<owner, name, as>>
			: std::bool_constant<!as.empty()> { };

		template <AggrFunc F, FixedString as>
		struct some_select_arg_with_as<AggrSelectArg<F, as>>
			: std::bool_constant<!as.empty()> { };
	}

	/**
	 * @concept senc::utils::sqlite::SomeSelectArg
	 * @brief Looks for any instantation of a select argument.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeSelectArg = sfinae::some_select_arg<Self>::value;

	/**
	 * @concept senc::utils::sqlite::SomeSelectArgWithOwner
	 * @brief Looks for any instantation of a select argument which has a table name.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeSelectArgWithOwner = sfinae::some_select_arg_with_owner<Self>::value;

	/**
	 * @concept senc::utils::SomeSelectArgWithAs
	 * @brief Looks for any instantation of a select argument which hass a rename column name.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept SomeSelectArgWithAs = sfinae::some_select_arg_with_as<Self>::value;

	namespace sfinae
	{
		// used for retrieving the containing table name from a select argument which has it
		template <SomeSelectArg Arg>
		struct select_arg_owner : EmptyFixedStringConstant { };

		template <SomeSelectArgWithOwner Arg>
		struct select_arg_owner<Arg> : FixedStringConstant<Arg::OWNER> { };
	}

	/**
	 * @var senc::utils::sqlite::SELECT_ARG_OWNER
	 * @brief Gets containing table name from select argument which has one.
	 * @tparam Arg Select argument.
	 * @note If `Arg` does not have a containing table name, evaluates to empty.
	 */
	template <SomeSelectArg Arg>
	constexpr FixedString SELECT_ARG_OWNER = sfinae::select_arg_owner<Arg>::value;

	/**
	 * @var senc::utils::sqlite::SELECT_ARG_NAME
	 * @brief Gets column name from select argument.
	 * @tparam Arg Select argument.
	 */
	template <SomeSelectArg Arg>
	constexpr FixedString SELECT_ARG_NAME = Arg::NAME;

	/**
	 * @var senc::utils::sqlite::SELECT_ARG_AS
	 * @brief Gets column rename column name from select argument which has one.
	 * @tparam Arg Select argument.
	 * @note If `Arg` does not have a rename column name, evaluates to empty.
	 */
	template <SomeSelectArg Arg>
	constexpr FixedString SELECT_ARG_AS = Arg::AS;
}

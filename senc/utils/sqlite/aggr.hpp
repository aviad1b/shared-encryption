/*********************************************************************
 * \file   aggr.hpp
 * \brief  Contains aggregate function typenames for sqlite utilities.
 *
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "../FixedString.hpp"
#include "values.hpp"

namespace senc::utils::sqlite
{
	/**
	 * @concept senc::utils::sqlite::AggrFunc
	 * @brief Looks for an sqlite aggregate function representing typename.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept AggrFunc = Value<typename Self::Output> &&
		requires
		{
			{ Self::NAME } -> FixedStringType;
			{ Self::COL } -> FixedStringType;
		};

	template <AggrFunc T>
	constexpr FixedString AGGR_FUNC_NAME = T::NAME;

	template <AggrFunc T>
	constexpr FixedString AGGR_FUNC_COL = T::COL;

	template <typename Self>
	concept AggrFuncWithAs = AggrFunc<Self> &&
		requires { { Self::AS } -> NonEmptyFixedStringType; };

	namespace sfinae
	{
		template <typename T>
		struct aggr_func_as : EmptyFixedStringConstant { };

		template <AggrFuncWithAs T>
		struct aggr_func_as<T> : FixedStringConstant<T::AS> { };
	}

	template <AggrFunc T>
	constexpr FixedString AGGR_FUNC_AS = sfinae::aggr_func_as<T>::value;

	template <typename Self>
	concept AggrFuncWithOwner = AggrFunc<Self> &&
		requires { { Self::OWNER } -> NonEmptyFixedStringType; };

	namespace sfinae
	{
		template <typename T>
		struct aggr_func_owner : EmptyFixedStringConstant { };

		template <AggrFuncWithOwner T>
		struct aggr_func_owner<T> : FixedStringConstant<T::OWNER> { };
	}

	template <AggrFuncWithOwner T>
	constexpr FixedString AGGR_FUNC_OWNER = sfinae::aggr_func_owner<T>::value;

	template <typename Self>
	concept AggrFuncWithAsAndOwner = AggrFuncWithAs<Self> && AggrFuncWithOwner<Self>;

	/**
	 * @struct senc::utils::sqlite::Avg
	 * @brief Represents the "avg" sqlite aggregation function.
	 * @tparam colName Column to apply aggregation function on.
	 */
	template <FixedString colName>
	struct Avg
	{
		static constexpr FixedString NAME = "AVG";
		static constexpr FixedString COL = colName;
		using Output = Real;
	};
	static_assert(AggrFunc<Avg<"column">>);

	/**
	 * @struct senc::utils::sqlite::Avg
	 * @brief Represents the "avg" sqlite aggregation function, with owning table name.
	 * @tparam owner Owning table name.
	 * @tparam colName Column to apply aggregation function on.
	 */
	template <FixedString owner, FixedString colName>
	struct OwnedAvg
	{
		static constexpr FixedString NAME = "AVG";
		static constexpr FixedString OWNER = owner;
		static constexpr FixedString COL = colName;
		using Output = Real;
	};

	/**
	 * @struct senc::utils::sqlite::Count
	 * @brief Represents the "count" sqlite aggregation function.
	 * @tparam colName Column to apply aggregation function on.
	 */
	template <FixedString colName>
	struct Count
	{
		static constexpr FixedString NAME = "COUNT";
		static constexpr FixedString COL = colName;
		using Output = Int;
	};
	static_assert(AggrFunc<Count<"column">>);

	/**
	 * @struct senc::utils::sqlite::Count
	 * @brief Represents the "count" sqlite aggregation function, with owning table name.
	 * @tparam owner Owning table name.
	 * @tparam colName Column to apply aggregation function on.
	 */
	template <FixedString owner, FixedString colName>
	struct OwnedCount
	{
		static constexpr FixedString NAME = "COUNT";
		static constexpr FixedString OWNER = owner;
		static constexpr FixedString COL = colName;
		using Output = Int;
	};
}

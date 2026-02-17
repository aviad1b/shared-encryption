/*********************************************************************
 * \file   TableView_impl.hpp
 * \brief  Implementation of sqlite TableView class.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#include "TableView.hpp"

#include "../AtScopeExit.hpp"
#include "sqlite_utils.hpp"

namespace senc::utils::sqlite
{
	template <schemas::SomeTable... Ts>
	class DatabaseUtils;

	template <FixedString name, schemas::SomeCol... Cs>
	class TableUtils;

	template <schemas::SomeCol C>
	class ColUtils;

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::TableView(sqlite3* db, std::string&& select)
		: _db(db), _select(std::move(select)) { }

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::TableView(
		sqlite3* db,
		const std::optional<std::string>& select,
		const std::optional<std::vector<std::string>>& where,
		const std::optional<std::vector<std::string>>& orderBy,
		const std::optional<std::int64_t> limit,
		const std::optional<std::int64_t> offset,
		const std::optional<std::function<std::string()>>& inner)
		: _db(db),
		  _select(select),
		  _where(where.value_or(std::vector<std::string>{})),
		  _orderBy(orderBy.value_or(std::vector<std::string>{})),
		  _limit(limit),
		  _offset(offset),
		  _inner(inner) { }

	template <schemas::SomeTable Schema>
	template <SomeSelectArg... Args>
	requires schemas::Selectable<
		Schema,
		Args...
	>
	inline TableView<schemas::Select<Schema, Args...>> TableView<Schema>::select() const
	{
		using RetSchema = schemas::Select<Schema, Args...>;
		using Ret = TableView<RetSchema>;

		// if has select already, return a new view with `this` being used as an inner view
		if (_select.has_value())
			return Ret(
				_db,
				std::string(schemas::TABLE_TO_SELECT<
					RetSchema,
					SelectArgsCollection<Args...>,
					false // disclude table name
				>),
				std::nullopt,
				std::nullopt,
				std::nullopt,
				std::nullopt,
				[*this]() -> std::string { return this->as_sql(); }
			);
		
		// otherwise, simply add select
		return Ret(
			_db,
			std::string(schemas::TABLE_TO_SELECT<
				RetSchema,
				SelectArgsCollection<Args...>
			>),
			_where,
			_orderBy,
			_limit,
			_offset,
			_inner
		);
	}

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::Self TableView<Schema>::where(const std::string& condition) const
	{
		using Ret = Self;

		Ret res = *this;
		res._where.push_back(condition);
		return res;
	}

	template <schemas::SomeTable Schema>
	template <SomeOrderArg Arg>
	inline TableView<Schema>::Self TableView<Schema>::order_by() const
	{
		using Ret = Self;

		Ret res = *this;
		res._orderBy.push_back(std::string(ORDER_ARG_NAME<Arg>) + " " + 
			std::string(ORDER_KIND_STR<ORDER_ARG_KIND<Arg>>));
		return res;
	}

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::Self TableView<Schema>::limit(std::int64_t n) const
	{
		using Ret = Self;

		// if isn't more restrictive than current limit, leave unchanged
		if (_limit.has_value() && n >= *_limit)
			return *this;

		// otherwise, apply new limit
		Ret res = *this;
		res._limit = n;
		return res;
	}

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::Self TableView<Schema>::offset(std::int64_t n) const
	{
		using Ret = Self;

		Ret res = *this;
		res._offset = res._offset.value_or(0) + n;
		return res;
	}

	template <schemas::SomeTable Schema>
	inline const TableView<Schema>::Self&
		TableView<Schema>::operator>>(Tuple& tpl) const
	{
		TableUtils(Schema{}).execute(
			_db, as_sql(),
			[&tpl](auto&&... values) { tpl = std::make_tuple(values...); },
			1
		);
		return *this;
	}

	template <schemas::SomeTable Schema>
	inline const TableView<Schema>::Self&
		TableView<Schema>::operator>>(std::vector<Tuple>& tpls) const
	{
		TableUtils(Schema{}).execute(
			_db, as_sql(),
			[&tpls](auto&&... values) { tpls.emplace_back(values...); },
			std::nullopt
		);
		return *this;
	}

	template <schemas::SomeTable Schema>
	inline const TableView<Schema>::Self&
		TableView<Schema>::operator>>(std::tuple_element_t<0, Tuple>& var) const
	requires (1 == ROW_LEN)
	{
		TableUtils(Schema{}).execute(
			_db, as_sql(),
			[&var](auto&& value) { var = value; },
			1
		);
		return *this;
	}

	template <schemas::SomeTable Schema>
	inline const TableView<Schema>::Self&
		TableView<Schema>::operator>>(std::vector<std::tuple_element_t<0, Tuple>>& vec) const
	requires (1 == ROW_LEN)
	{
		TableUtils(Schema{}).execute(
			_db, as_sql(),
			[&vec](auto&& value) { vec.emplace_back(value); },
			std::nullopt
		);
		return *this;
	}

	template <schemas::SomeTable Schema>
	inline const TableView<Schema>::Self&
		TableView<Schema>::operator>>(schemas::TableCallable<Schema> auto&& callback) const
	{
		TableUtils(Schema{}).execute(_db, as_sql(), callback, std::nullopt);
		return *this;
	}

	template <schemas::SomeTable Schema>
	inline std::string TableView<Schema>::as_sql() const
	{
		std::string res = _select.has_value() ? *_select : "SELECT * FROM";

		if (_inner.has_value())
			res += "(" + (*_inner)() + ")";

		if (!_where.empty())
		{
			res += " WHERE ";
			for (const auto& clause : _where | std::views::take(_where.size() - 1))
				res += clause + " AND ";
			res += _where.back();
		}

		if (!_orderBy.empty())
		{
			res += " ORDER BY ";
			for (const auto& clause : _orderBy | std::views::take(_orderBy.size() - 1))
				res += clause + ", ";
			res += _orderBy.back();
		}

		if (_limit.has_value())
			res += " LIMIT " + std::to_string(*_limit);

		if (_offset.has_value())
		{
			// offset without limit is invalid, if has offset but not limit use limit -1 (no limit)
			if (!_limit.has_value())
				res += " LIMIT -1";
			res += " OFFSET " + std::to_string(*_offset);
		}

		return res;
	}
}

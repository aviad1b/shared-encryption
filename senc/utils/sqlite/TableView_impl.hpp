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
	inline TableView<Schema>::TableView(sqlite3* db)
		: _db(db), _select(schemas::TABLE_TO_SELECT<Schema>) { }

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::TableView(
		sqlite3* db,
		const std::optional<std::string>& select,
		const std::optional<std::vector<std::string>>& where,
		const std::optional<std::function<std::string()>>& inner)
		: _db(db), _select(select), _where(where), _inner(inner) { }

	template <schemas::SomeTable Schema>
	template <SomeSelectArg... Args>
	requires schemas::Selectable<
		Schema,
		Args...
	>
	inline TableView<schemas::Select<Schema, Args...>> TableView<Schema>::select()
	{
		using RetSchema = schemas::Select<Schema, Args...>;
		using Ret = TableView<RetSchema>;

		// if has select already, return a new view with `this` being used as an inner view
		if (_select.has_value())
			return Ret(
				_db,
				std::nullopt,
				std::nullopt,
				[*this] { return this->as_sql(); }
			);
		
		// otherwise, simply add select
		return Ret(
			_db,
			schemas::TABLE_TO_SELECT<RetSchema>,
			std::nullopt,
			std::nullopt
		);
	}

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::Self TableView<Schema>::where(const std::string& condition)
	{
		using Ret = Self;

		Ret res = *this;
		res._where.push_back(condition);
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
			res += " (" + (*_inner)() + ")";
		if (!_where.empty())
		{
			res += " WHERE ";
			for (const auto& clause : _where | std::views::take(_where.size() - 1))
				res += clause + " AND ";
			res += _where.back();
		}
		return res;
	}
}

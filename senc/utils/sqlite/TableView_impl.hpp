/*********************************************************************
 * \file   TableView_impl.hpp
 * \brief  Implementation of sqlite TableView class.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#include "TableView.hpp"

namespace senc::utils::sqlite
{
	template <schemas::SomeTable Schema>
	inline TableView<Schema>::TableView(sqlite3* db)
		: _db(db), _select(schemas::TABLE_TO_SELECT<Schema>) { }

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

		// if  has select already, return a new view with `this` being used as an inner view
		if (!_select.has_value())
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
	template <FixedString... cols>
	inline TableView<Schema>::Self TableView<Schema>::where(const std::string& condition)
	{
		using Ret = Self;

		Ret res = *this;
		res._where.push_back(condition);
		return res;
	}

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::Self&
		TableView<Schema>::operator>>(Tuple& tpl)
	{
		this->execute(
			[&tpl](auto&&... values) { tpl = std::make_tuple(values...); },
			1
		);
		return *this;
	}

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::Self&
		TableView<Schema>::operator>>(std::tuple_element_t<0, Tuple>& var)
	requires (1 == ROW_LEN)
	{
		this->execute(
			[&var](auto&& value) { var = value; },
			1
		);
		return *this;
	}

	template <schemas::SomeTable Schema>
	inline TableView<Schema>::Self&
		TableView<Schema>::operator>>(schemas::TableCallable<Schema> auto&& callback)
	{
		this->execute(callback, std::nullopt);
		return *this;
	}

	template <schemas::SomeTable Schema>
	inline void TableView<Schema>::execute(
		schemas::TableCallable<Schema> auto&& callback,
		std::optional<int> limit)
	{
		sqlite3_stmt* stmt = nullptr;
		const auto sql = as_sql();
		if (SQLITE_OK != sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr))
			throw SQLiteException("Failed to run statement: " + sql);

		// if has limit, set limit function to compare; otherwise, limit function always false
		auto pastLimit = limit.has_value() ? [limit](int i) { return i >= *limit; }
			: [](int) { return false; };

		for (int i = 0; SQLITE_ROW == sqlite3_step(stmt); ++i)
		{
			if (pastLimit(i))
				throw SQLiteException("Too many rows to unpack: Expected " + std::to_string(*limit));

			execute_any(callback, stmt);
		}
	}

	template <schemas::SomeTable Schema>
	template <FixedString name, schemas::SomeCol... Cs, std::size_t... is>
	inline void TableView<Schema>::execute_util(
		schemas::TableCallable<schemas::Table<name, Cs...>> auto&& callback,
		sqlite3_stmt* stmt)
	{
		// for each column C with index i,
		// construct a view of that column from stmt and i
		callback(schemas::ColView<Cs>(stmt, is)...);
	}
}

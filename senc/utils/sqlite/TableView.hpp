/*********************************************************************
 * \file   TableView.hpp
 * \brief  Header of sqlite TableView class.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "schemas/all.hpp"
#include <optional>
#include <string>
#include <vector>

namespace senc::utils::sqlite
{
	/**
	 * @class senc::utils::sqlite::TableView
	 * @brief Used to view a database's table (or joined tables).
	 * @tparam Schema Schema of viewed table.
	 * @note Has forward declaration in `sqlite_utils.hpp`
	 */
	template <schemas::SomeTable Schema>
	class TableView
	{
	public:
		using Self = TableView<Schema>;
		using Tuple = schemas::TableTuple<Schema>;
		static constexpr auto ROW_LEN = std::tuple_size_v<Tuple>;
		
		/**
		 * @brief Copy constructor of table view.
		 */
		TableView(const Self&) = default;

		/**
		 * @brief Copy assignment operator of table view.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Constructs a table view form native sqlite3 pointer.
		 * @param db Native sqlite3 pointer.
		 */
		explicit TableView(sqlite3* db);

		/**
		 * @brief Constructs a table view from native sqlite3 pointer and an inner query.
		 * @param db Native sqlite3 pointer.
		 * @param inner A (lambda) function returning an inner query (using a copied inner view).
		 */
		explicit TableView(sqlite3* db,
						   const std::optional<std::string>& select,
						   const std::optional<std::vector<std::string>>& where,
						   const std::optional<std::function<std::string()>>& inner);
		
		/**
		 * @brief Applies a select query on the table view.
		 * @tparam Args Select arguments.
		 * @see senc::utils::sqlite::SelectArg
		 * @return Table view with "select" applied.
		 */
		template <SomeSelectArg... Args>
		requires schemas::Selectable<
			Schema,
			Args...
		>
		TableView<schemas::Select<Schema, Args...>> select();

		/**
		 * @brief Applies a where clause on the table view.
		 * @return Table view with "where" applied.
		 */
		Self where(const std::string& condition);

		/**
		 * @brief Outputs viewed data into a fitting tuple.
		 * @param tpl Tuple to output into.
		 * @return `*this`.
		 */
		const Self& operator>>(Tuple& tpl) const;

		/**
		 * @brief Outputs viewed data into a fitting variable.
		 * @note Requires schema to contain one value exactly.
		 * @param var Variable to output into.
		 * @return `*this`.
		 */
		const Self& operator>>(std::tuple_element_t<0, Tuple>& var) const
		requires (1 == ROW_LEN);

		/**
		 * @brief Outputs viewed data into a fitting callback functor.
		 * @param callback Callback functor to output into.
		 * @return `*this`.
		 */
		const Self& operator>>(schemas::TableCallable<Schema> auto&& callback) const;

	private:
		sqlite3* _db;
		std::optional<std::string> _select;
		std::vector<std::string> _where;
		std::optional<std::function<std::string()>> _inner;

		/**
		 * @brief Gets SQL query version of table view.
		 * @return SQL query version of table view.
		 */
		std::string as_sql() const;
	};
}

#include "TableView_impl.hpp"

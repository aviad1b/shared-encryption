/*********************************************************************
 * \file   values_impl.hpp
 * \brief  Template implementation of sqlite values utilities.
 * 
 * \author aviad1b
 * \date   January 2026, SHvat 5786
 *********************************************************************/

#include "values.hpp"

namespace senc::utils::sqlite
{
	template <typename Ret, typename... Args>
	inline Ret ValueViewTag::ValueViewData::exec(
		Ret(*valueFunc)(sqlite3_value*),
		Ret(*columnFunc)(sqlite3_stmt*, int)) const
	{
		return std::visit([valueFunc, columnFunc](auto&& arg)
		{
			using T = std::remove_cvref_t<decltype(arg)>;
			if constexpr (std::same_as<T, ColumnData>)
				return columnFunc(arg.stmt, arg.col);
			else return valueFunc(arg);
		}, _data);
	}

	template <Value T>
	inline NullableView<T>::NullableView() : _view(std::nullopt) { }

	template <Value T>
	inline NullableView<T>::NullableView(ValueViewData&& data)
		: _view((SQLITE_NULL == data.exec(sqlite3_value_type, sqlite3_column_type)) ? std::nullopt
				: std::optional<ValueViewOf<T>>(std::move(data))) { }

	template <Value T>
	inline NullableView<T>::NullableView(sqlite3_value* value) : Self(ValueViewData(value)) { }

	template <Value T>
	inline NullableView<T>::NullableView(sqlite3_stmt* stmt, int col) : Self(ValueViewData(stmt, col)) { }

	template <Value T>
	inline bool NullableView<T>::has_value() const
	{
		return _view.has_value();
	}

	template <Value T>
	inline bool NullableView<T>::is_null() const
	{
		return !has_value();
	}

	template <Value T>
	inline NullableView<T>::operator bool() const
	{
		return has_value();
	}

	template <Value T>
	inline bool NullableView<T>::operator!() const
	{
		return is_null();
	}

	template <Value T>
	inline const ValueViewOf<T>& NullableView<T>::operator*() const
	{
		return *_view;
	}

	template <Value T>
	inline const ValueViewOf<T>* NullableView<T>::operator->() const
	{
		return std::to_address(_view);
	}

	template <Value T>
	inline const std::optional<ValueViewOf<T>>& NullableView<T>::get() const
	{
		return _view;
	}

	template <Value T>
	inline NullableView<T>::operator const std::optional<ValueViewOf<T>>&() const
	{
		return get();
	}

	template <Value T>
	inline Nullable<T>::Nullable(std::nullopt_t) : _value(std::nullopt) { }

	template <Value T>
	template <typename... Args>
	requires std::constructible_from<T, Args...>
	inline Nullable<T>::Nullable(Args&&... args)
		: _value(std::forward<Args>(args)...) { }

	template <Value T>
	inline Nullable<T>::Nullable(const View& view) : _value(view.get()) { }

	template <Value T>
	inline bool Nullable<T>::has_value() const
	{
		return _value.has_value();
	}

	template <Value T>
	inline bool Nullable<T>::is_null() const
	{
		return !has_value();
	}

	template <Value T>
	inline Nullable<T>::operator bool() const
	{
		return has_value();
	}

	template <Value T>
	inline bool Nullable<T>::operator!() const
	{
		return is_null();
	}

	template <Value T>
	inline const T& Nullable<T>::operator*() const
	{
		return *_value;
	}

	template <Value T>
	inline const T* Nullable<T>::operator->() const
	{
		return std::to_address(_value);
	}

	template <Value T>
	inline const std::optional<T>& Nullable<T>::get() const
	{
		return _value;
	}

	template <Value T>
	inline Nullable<T>::operator const std::optional<T>&() const
	{
		return get();
	}

	template <Value T>
	inline std::string Nullable<T>::as_sqlite() const
	{
		return _value.value_or(Null{}).as_sqlite();
	}

	template <Value T>
	inline void Nullable<T>::bind(sqlite3_stmt* stmt, int index) const
	{
		_value.value_or(Null{}).bind(stmt, index);
	}

	template <typename... Args>
	requires std::constructible_from<Buffer, Args...>
	inline Blob::Blob(Args&&... args) : _value(std::forward<Args>(args)...) { }
}

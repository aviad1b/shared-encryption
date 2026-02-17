/*********************************************************************
 * \file   values.hpp
 * \brief  Header of sqlite values utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include "SQLiteException.hpp"
#include "../FixedString.hpp"
#include "../variants.hpp"
#include "../concepts.hpp"
#include "../bytes.hpp"
#include <sqlite3.h>
#include <optional>
#include <utility>
#include <sstream>
#include <string>

namespace senc::utils::sqlite
{
	/**
	 * @struct senc::utils::sqlite::ValueViewTag
	 * @brief Parent "tag" struct for SQL value views.
	 */
	struct ValueViewTag
	{
	protected:
		/**
		 * @class senc::utils::sqlite::ValueViewTag::ValueViewData
		 * @brief Encapsulates storage of an SQL value pointer.
		 */
		class ValueViewData
		{
		public:
			using Self = ValueViewData;

			/**
			 * @brief Constructs from a pointer provided by a callback function.
			 * @param value Value pointer provided by a callback function.
			 */
			ValueViewData(sqlite3_value* value);

			/**
			 * @brief Constructs from statement pointer and column index.
			 * @param stmt Statement pointer.
			 * @param col Column index.
			 */
			ValueViewData(sqlite3_stmt* stmt, int col);

			/**
			 * @brief Executes a native SQLite function on the stored pointer.
			 * @param valueFunc Function to run if has `sqlite3_value*`.
			 * @param columnFunc Function to run if has `sqlite3_stmt*` and column index.
			 */
			template <typename Ret, typename... Args>
			Ret exec(Ret(*valueFunc)(sqlite3_value*),
				Ret(*columnFunc)(sqlite3_stmt*, int)) const;

		private:
			struct ColumnData { sqlite3_stmt* stmt; int col; };
			std::variant<sqlite3_value*, ColumnData> _data;
		};
	};

	/**
	 * @concept senc::utils::sqlite::ValueView
	 * @brief Looks for an SQLite value view type.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ValueView = std::derived_from<Self, ValueViewTag> &&
		std::constructible_from<Self, sqlite3_value*> &&
		std::constructible_from<Self, sqlite3_stmt*, int> &&
		requires(const Self self, sqlite3_stmt* stmt, int index)
		{
			{ std::bool_constant<Self::is_nullable()>() }; // must be constexpr-evaluable
			{ self.bind(stmt, index) };
		};

	/**
	 * @struct senc::utils::sqlite::ValueTag
	 * @brief Parent "tag" class for SQL values.
	 */
	struct ValueTag { };

	/**
	 * @concept senc::utils::sqlite::Value
	 * @brief Looks for an SQLite value type.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Value = std::derived_from<Self, ValueTag> &&
		ValueView<typename Self::View> &&
		std::constructible_from<Self, typename Self::View> &&
		requires(const Self self, sqlite3_value* value, sqlite3_stmt* stmt, int index)
		{
			typename Self::BasedOn;
			{ std::bool_constant<Self::is_nullable()>() }; // must be constexpr-evaluable
			{ FixedStringConstant<Self::SQL_TYPE>{} }; // must be constexpr
			{ self.as_sqlite() } -> std::convertible_to<std::string>;
			{ self.bind(stmt, index) };
		};

	/**
	 * @typedef senc::utils::sqlite::ValueViewOf
	 * @brief SQL value view type of a given value type.
	 * @tparam V SQL value type.
	 */
	template <Value V>
	using ValueViewOf = typename V::View;

	namespace sfinae
	{
		// used for converting a ValueView to a Value
		template <ValueView VV>
		struct value_of_view { };
	}

	template <ValueView VV>
	using ValueOfView = typename sfinae::value_of_view<VV>::type;

	/**
	 * @class senc::utils::NullView
	 * @brief Views an SQL NULL value.
	 */
	class NullView : public ValueViewTag
	{
	public:
		using Self = NullView;

		/**
		 * @brief Default constructor of SQL NULL view.
		 */
		NullView();

		/**
		 * @brief Constructs an SQL NULL view from value view data.
		 * @param data Value view data (moved).
		 */
		NullView(ValueViewData&& data);

		/**
		 * @brief Constructs an SQL NULL view from `sqlite3_value*`.
		 * @param value An `sqlite3_value*`.
		 */
		NullView(sqlite3_value* value);

		/**
		 * @brief Constructs an SQL NULL view from `sqlite3_stmt*` and a column index.
		 * @param stmt An `sqlite3_stmt*`.
		 * @param col Column index.
		 */
		NullView(sqlite3_stmt* stmt, int col);

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		std::nullopt_t get() const;

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		operator std::nullopt_t() const;

		/**
		 * @brief Checks if view's data can have null.
		 * @return `true`.
		 */
		static constexpr bool is_nullable() { return true; }

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		std::optional<ValueViewData> _data;
	};
	static_assert(ValueView<NullView>);

	/**
	 * @class senc::utils::sqlite::Null
	 * @brief Represents an SQL NULL value.
	 */
	class Null : public ValueTag
	{
	public:
		using Self = Null;
		using View = NullView;
		using BasedOn = std::nullopt_t;
		static constexpr FixedString SQL_TYPE = "NULL";

		/**
		 * @brief Default constructor of SQL NULL value (set NULL).
		 */
		Null() = default;

		/**
		 * @brief Constructs an SQL NULL value from `std::nullopt`.
		 */
		Null(std::nullopt_t) : Self() { }

		/**
		 * @brief Constructs an SQL NULL value from an SQL NULL view.
		 */
		Null(const View&) : Self() { }

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		std::nullopt_t get() const;

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		operator std::nullopt_t() const;

		/**
		 * @brief Checks if stored data can have null.
		 * @return `true`.
		 */
		static constexpr bool is_nullable() { return true; }

		/**
		 * @brief Gets SQLite representation of value.
		 * @return SQLite representation of value.
		 */
		std::string as_sqlite() const;

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;
	};
	namespace sfinae
	{
		template <>
		struct value_of_view<NullView> { using type = Null; };
	}
	static_assert(Value<Null>);
	static_assert(std::same_as<ValueViewOf<Null>, NullView>);
	static_assert(std::same_as<ValueOfView<NullView>, Null>);

	/**
	 * @class senc::utils::sqlite::NullableView
	 * @brief Views an SQL value that may contain NULL.
	 * @tparam T Value type.
	 */
	template <Value T>
	class NullableView : public ValueViewTag
	{
	public:
		using Self = NullableView<T>;

		/**
		 * @brief Default constructor of nullable view (initializes to NULL).
		 */
		NullableView();

		/**
		 * @brief Constructs a nullable SQL view from value view data.
		 * @param data Value view data (moved).
		 */
		NullableView(ValueViewData&& data);

		/**
		 * @brief Constructs a nullable SQL view from `sqlite3_value*`.
		 * @param value An `sqlite3_value*`.
		 */
		NullableView(sqlite3_value* value);

		/**
		 * @brief Constructs a nullable SQL view from `sqlite3_stmt*` and a column index.
		 * @param stmt An `sqlite3_stmt*`.
		 * @param col Column index.
		 */
		NullableView(sqlite3_stmt* stmt, int col);

		/**
		 * @brief Checks if has a non-null value.
		 * @return `true` if has a non-null value, otherwise `false`.
		 */
		bool has_value() const;

		/**
		 * @brief Checks if has null.
		 * @return `true` if has null, otherwise `false`.
		 */
		bool is_null() const;

		/**
		 * @brief Checks if has a non-null value.
		 * @return `true` if has a non-null value, otherwise `false`.
		 */
		operator bool() const;

		/**
		 * @brief Checks if has null.
		 * @return `true` if has null, otherwise `false`.
		 */
		bool operator!() const;

		/**
		 * @brief Gets a view of the contained value, if not null.
		 * @return View of the contained value.
		 */
		const ValueViewOf<T>& operator*() const;

		/**
		 * @brief Gets pointer to view of the contained value, if not null.
		 * @return Pointer to view of the container value.
		 */
		const ValueViewOf<T>* operator->() const;

		/**
		 * @brief Gets view of the container value.
		 * @return View of the container value, or `std::nullopt` if null.
		 */
		const std::optional<ValueViewOf<T>>& get() const;

		/**
		 * @brief Gets view of the container value.
		 * @return View of the container value, or `std::nullopt` if null.
		 */
		operator const std::optional<ValueViewOf<T>>&() const;

		/**
		 * @brief Checks if view's data can have null.
		 * @return `true`.
		 */
		static constexpr bool is_nullable() { return true; }

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		std::optional<ValueViewOf<T>> _view;
	};

	/**
	 * @class senc::utils::sqlite::Nullable
	 * @brief Represents an SQL value which may be NULL.
	 */
	template <Value T>
	class Nullable : public ValueTag
	{
	public:
		using Self = Nullable<T>;
		using View = NullableView<T>;
		using BasedOn = typename T::BasedOn;
		static constexpr FixedString SQL_TYPE = T::SQL_TYPE;

		/**
		 * @brief Constructs a nullable with null.
		 */
		Nullable(std::nullopt_t);

		/**
		 * @brief Constructs a nullable with non-null value.
		 * @param args Arguments to construct value from.
		 */
		template <typename... Args>
		requires std::constructible_from<T, Args...>
		Nullable(Args&&... args);

		/**
		 * @brief Constructs an SQL nullable value from an SQL nullable view.
		 */
		Nullable(const View& view);

		/**
		 * @brief Checks if has a non-null value.
		 * @return `true` if has a non-null value, otherwise `false`.
		 */
		bool has_value() const;

		/**
		 * @brief Checks if has null.
		 * @return `true` if has null, otherwise `false`.
		 */
		bool is_null() const;

		/**
		 * @brief Checks if has a non-null value.
		 * @return `true` if has a non-null value, otherwise `false`.
		 */
		operator bool() const;

		/**
		 * @brief Checks if has null.
		 * @return `true` if has null, otherwise `false`.
		 */
		bool operator!() const;

		/**
		 * @brief Gets contained value (if not null).
		 * @return Contained value.
		 */
		const T& operator*() const;
		
		/**
		 * @brief Gets pointer to contained value (if not null).
		 * @return Pointer to contained value.
		 */
		const T* operator->() const;

		/**
		 * @brief Gets contained value.
		 * @return Contained value, or `std::nullopt` if null.
		 */
		const std::optional<T>& get() const;

		/**
		 * @brief Gets contained value.
		 * @return Contained value, or `std::nullopt` if null.
		 */
		operator const std::optional<T>&() const;

		/**
		 * @brief Checks if stored data can have null.
		 * @return `true`.
		 */
		static constexpr bool is_nullable() { return true; }

		/**
		 * @brief Gets SQLite representation of value.
		 * @return SQLite representation of value.
		 */
		std::string as_sqlite() const;

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		std::optional<T> _value;
	};
	namespace sfinae
	{
		template <Value T>
		struct value_of_view<NullableView<T>> { using type = Nullable<T>; };
	}

	/**
	 * @class senc::utils::sqlite::IntView
	 * @brief Views an SQL int value.
	 */
	class IntView : public ValueViewTag
	{
	public:
		using Self = IntView;

		/**
		 * @brief Constructs an SQL int view from value view data.
		 * @param data Value view data (moved).
		 */
		IntView(ValueViewData&& data);

		/**
		 * @brief Constructs an SQL int view from `sqlite3_value*`.
		 * @param value An `sqlite3_value*`.
		 */
		IntView(sqlite3_value* value);

		/**
		 * @brief Constructs an SQL int view from `sqlite3_stmt*` and a column index.
		 * @param stmt An `sqlite3_stmt*`.
		 * @param col Column index.
		 */
		IntView(sqlite3_stmt* stmt, int col);

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		std::int64_t get() const;

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		operator std::int64_t() const;

		/**
		 * @brief Checks if view's data can have null.
		 * @return `false`.
		 */
		static constexpr bool is_nullable() { return false; }

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		ValueViewData _data;
	};
	static_assert(ValueView<IntView>);

	/**
	 * @class senc::utils::sqlite::Int
	 * @brief Represents an SQL int value.
	 */
	class Int : public ValueTag
	{
	public:
		using Self = Int;
		using View = IntView;
		using BasedOn = std::int64_t;
		static constexpr FixedString SQL_TYPE = "INT";

		/**
		 * @brief Default constructor of SQL int value (sets 0).
		 */
		Int();

		/**
		 * @brief Constructs an SQL int value from a given int value.
		 * @param value Int value.
		 */
		Int(std::int64_t value);

		/**
		 * @brief Constructs an SQL int value from an SQL int view.
		 * @param view SQL int view.
		 */
		Int(const View& view);

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		std::int64_t get() const;

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		operator std::int64_t() const;

		/**
		 * @brief Checks if stored data can have null.
		 * @return `false`.
		 */
		static constexpr bool is_nullable() { return false; }

		/**
		 * @brief Gets SQLite representation of value.
		 * @return SQLite representation of value.
		 */
		std::string as_sqlite() const;

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		sqlite3_int64 _value;
	};
	namespace sfinae
	{
		template <>
		struct value_of_view<IntView> { using type = Int; };
	}
	static_assert(Value<Int>);
	static_assert(Value<Nullable<Int>>);
	static_assert(ValueView<NullableView<Int>>);
	static_assert(std::same_as<ValueViewOf<Int>, IntView>);
	static_assert(std::same_as<ValueOfView<IntView>, Int>);
	static_assert(std::same_as<ValueViewOf<Nullable<Int>>, NullableView<Int>>);
	static_assert(std::same_as<ValueOfView<NullableView<Int>>, Nullable<Int>>);

	/**
	 * @class senc::utils::sqlite::RealView
	 * @brief Views an SQL real value.
	 */
	class RealView : public ValueViewTag
	{
	public:
		using Self = RealView;

		/**
		 * @brief Constructs an SQL real view from value view data.
		 * @param data Value view data (moved).
		 */
		RealView(ValueViewData&& data);

		/**
		 * @brief Constructs an SQL real view from `sqlite3_value*`.
		 * @param value An `sqlite3_value*`.
		 */
		RealView(sqlite3_value* value);

		/**
		 * @brief Constructs an SQL real view from `sqlite3_stmt*` and a column index.
		 * @param stmt An `sqlite3_stmt*`.
		 * @param col Column index.
		 */
		RealView(sqlite3_stmt* stmt, int col);

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		double get() const;

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		operator double() const;

		/**
		 * @brief Checks if view's data can have null.
		 * @return `false`.
		 */
		static constexpr bool is_nullable() { return false; }

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		ValueViewData _data;
	};
	static_assert(ValueView<RealView>);

	/**
	 * @class senc::utils::sqlite::Real
	 * @brief Represents an SQL real value.
	 */
	class Real : public ValueTag
	{
	public:
		using Self = Real;
		using View = RealView;
		using BasedOn = double;
		static constexpr FixedString SQL_TYPE = "REAL";

		/**
		 * @brief Default constructor of SQL real value (sets 0).
		 */
		Real();

		/**
		 * @brief Constructs an SQL real value from a given double value.
		 * @param value Double value.
		 */
		Real(double value);

		/**
		 * @brief Constructs an SQL real value from an SQL real view.
		 * @param view SQL real view.
		 */
		Real(const View& view);

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		double get() const;

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		operator double() const;

		/**
		 * @brief Checks if stored data can have null.
		 * @return `false`.
		 */
		static constexpr bool is_nullable() { return false; }

		/**
		 * @brief Gets SQLite representation of value.
		 * @return SQLite representation of value.
		 */
		std::string as_sqlite() const;

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		double _value;
	};
	namespace sfinae
	{
		template <>
		struct value_of_view<RealView> { using type = Real; };
	}
	static_assert(Value<Real>);
	static_assert(Value<Nullable<Real>>);
	static_assert(ValueView<NullableView<Real>>);
	static_assert(std::same_as<ValueViewOf<Real>, RealView>);
	static_assert(std::same_as<ValueOfView<RealView>, Real>);
	static_assert(std::same_as<ValueViewOf<Nullable<Real>>, NullableView<Real>>);
	static_assert(std::same_as<ValueOfView<NullableView<Real>>, Nullable<Real>>);

	/**
	 * @class senc::utils::sqlite::TextView
	 * @brief Views an SQL text value.
	 */
	class TextView : public ValueViewTag
	{
	public:
		using Self = TextView;

		/**
		 * @brief Constructs an SQL text view from an existing string view.
		 * @param view Existing string view.
		 */
		TextView(std::string_view view);

		/**
		 * @brief Constructs an SQL text view from value view data.
		 * @param data Value view data (moved).
		 */
		TextView(ValueViewData&& data);

		/**
		 * @brief Constructs an SQL text view from `sqlite3_value*`.
		 * @param value An `sqlite3_value*`.
		 */
		TextView(sqlite3_value* value);

		/**
		 * @brief Constructs an SQL text view from `sqlite3_stmt*` and a column index.
		 * @param stmt An `sqlite3_stmt*`.
		 * @param col Column index.
		 */
		TextView(sqlite3_stmt* stmt, int col);

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		std::string_view get() const;

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		operator std::string_view() const;

		/**
		 * @brief Checks if view's data can have null.
		 * @return `false`.
		 */
		static constexpr bool is_nullable() { return false; }

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		std::variant<ValueViewData, std::string_view> _data;
	};
	static_assert(ValueView<TextView>);

	/**
	 * @class senc::utils::sqlite::Text
	 * @brief Represents an SQL text value.
	 */
	class Text : public ValueTag
	{
	public:
		using Self = Text;
		using View = TextView;
		using BasedOn = std::string;
		static constexpr FixedString SQL_TYPE = "TEXT";

		/**
		 * @brief Default constructor of SQL int value (sets empty string).
		 */
		Text() = default;

		/**
		 * @brief Constructs an SQL text value from a given string value.
		 * @param value String value.
		 */
		Text(const std::string& value);

		/**
		 * @brief Constructs an SQL text value from a given (moved) string value.
		 * @param value String value (moved).
		 */
		Text(std::string&& value);

		/**
		 * @brief Constructs an SQL text value from an SQL text view.
		 * @param view SQL text view.
		 */
		Text(const View& view);

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		const std::string& get() const;

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		operator const std::string&() const;

		/**
		 * @brief Checks if stored data can have null.
		 * @return `false`.
		 */
		static constexpr bool is_nullable() { return false; }

		/**
		 * @brief Gets SQLite representation of value.
		 * @return SQLite representation of value.
		 */
		std::string as_sqlite() const;

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		std::string _value;
	};
	namespace sfinae
	{
		template <>
		struct value_of_view<TextView> { using type = Text; };
	}
	static_assert(Value<Text>);
	static_assert(Value<Nullable<Text>>);
	static_assert(ValueView<NullableView<Text>>);
	static_assert(std::same_as<ValueViewOf<Text>, TextView>);
	static_assert(std::same_as<ValueOfView<TextView>, Text>);
	static_assert(std::same_as<ValueViewOf<Nullable<Text>>, NullableView<Text>>);
	static_assert(std::same_as<ValueOfView<NullableView<Text>>, Nullable<Text>>);

	/**
	 * @class senc::utils::sqlite::BlobView
	 * @brief Views an SQL blob value.
	 */
	class BlobView : public ValueViewTag
	{
	public:
		using Self = BlobView;

		/**
		 * @brief Constructs an SQL blob view from an existing bytes view.
		 * @param view Existing bytes view.
		 */
		BlobView(BytesView view);

		/**
		 * @brief Constructs an SQL blob view from value view data.
		 * @param data Value view data (moved).
		 */
		BlobView(ValueViewData&& data);

		/**
		 * @brief Constructs an SQL blob view from `sqlite3_value*`.
		 * @param value An `sqlite3_value*`.
		 */
		BlobView(sqlite3_value* value);

		/**
		 * @brief Constructs an SQL blob view from `sqlite3_stmt*` and a column index.
		 * @param stmt An `sqlite3_stmt*`.
		 * @param col Column index.
		 */
		BlobView(sqlite3_stmt* stmt, int col);

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		BytesView get() const;

		/**
		 * @brief Gets viewed data.
		 * @return Viewed data.
		 */
		operator BytesView() const;

		/**
		 * @brief Checks if view's data can have null.
		 * @return `false`.
		 */
		static constexpr bool is_nullable() { return false; }

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		std::variant<ValueViewData, BytesView> _data;
	};
	static_assert(ValueView<BlobView>);

	/**
	 * @class senc::utils::sqlite::Blob
	 * @brief Represents an SQL blob value.
	 */
	class Blob : public ValueTag
	{
	public:
		using Self = Blob;
		using View = BlobView;
		using BasedOn = Buffer;
		static constexpr FixedString SQL_TYPE = "BLOB";

		/**
		 * @brief Default constructor of SQL int value (sets empty).
		 */
		Blob() = default;

		/**
		 * @brief Constructs an SQL blob value from a given binary value.
		 * @param value Binary value (moved buffer).
		 */
		Blob(Buffer&& value);

		/**
		 * @brief Constructs an SQL blob value from initializer list of bytes.
		 * @param bytes Initialized list of bytes.
		 */
		Blob(std::initializer_list<byte> bytes);

		/**
		 * @brief Constructs an SQL blob value from an SQL blob view.
		 * @param view SQL blob view.
		 */
		Blob(const View& view);

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		const Buffer& get() const;

		/**
		 * @brief Gets stored data.
		 * @return Stored data.
		 */
		operator const Buffer&() const;

		/**
		 * @brief Checks if stored data can have null.
		 * @return `false`.
		 */
		static constexpr bool is_nullable() { return false; }

		/**
		 * @brief Gets SQLite representation of value.
		 * @return SQLite representation of value.
		 */
		std::string as_sqlite() const;

		/**
		 * @brief Binds as parameter to a given statement.
		 * @param stmt Statement pointer.
		 * @param index Param index to bind to.
		 */
		void bind(sqlite3_stmt* stmt, int index) const;

	private:
		Buffer _value;
	};
	namespace sfinae
	{
		template <>
		struct value_of_view<BlobView> { using type = Blob; };
	}
	static_assert(Value<Blob>);
	static_assert(Value<Nullable<Blob>>);
	static_assert(ValueView<NullableView<Blob>>);
	static_assert(std::same_as<ValueViewOf<Blob>, BlobView>);
	static_assert(std::same_as<ValueOfView<BlobView>, Blob>);
	static_assert(std::same_as<ValueViewOf<Nullable<Blob>>, NullableView<Blob>>);
	static_assert(std::same_as<ValueOfView<NullableView<Blob>>, Nullable<Blob>>);
}

#include "values_impl.hpp"

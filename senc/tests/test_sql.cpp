/*********************************************************************
 * \file   test_sql.cpp
 * \brief  Contains tests for SQL-related utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <filesystem>
#include <optional>
#include <tuple>

#include "../utils/sqlite/Database.hpp"

namespace sql = senc::utils::sqlite;

class SqlTest : public testing::Test
{
protected:
	// schema: Users(id: INT PK, name: TEXT, age: REAL, data: BLOB?)
	//         FavNumbers(user_id: INT FK[Users.id], fav_num: Int)
	std::optional<sql::TempDatabase<sql::schemas::DB<
		sql::schemas::Table<"Users",
			sql::schemas::PrimaryKey<"id"  , sql::Int                >,
			sql::schemas::Col       <"name", sql::Text               >,
			sql::schemas::Col       <"age" , sql::Real               >,
			sql::schemas::Col       <"data", sql::Nullable<sql::Blob>>
		>,
		sql::schemas::Table<"FavNumbers",
			sql::schemas::ForeignKey<"user_id", sql::Int, "Users", "id">,
			sql::schemas::Col       <"fav_num", sql::Int               >
		>
	>>> db;

	void SetUp() override
	{
		const std::string path = "database.sqlite";
		if (std::filesystem::exists(path))
			std::remove(path.c_str());
		db.emplace(path);

		db->insert<"Users">(1, "Avi",   22.0,  std::nullopt);
		db->insert<"Users">(2, "Batya", 18.5,  senc::utils::Buffer{ 0xAA, 0xBB, 0xCC });

		db->insert<"FavNumbers">(1, 434);
		db->insert<"FavNumbers">(2, 256);
	}

	void TearDown() override
	{
		db.reset();
	}
};

// ---------------------------------------------------------------------------
// select + where + operator>> (single scalar)
// ---------------------------------------------------------------------------

// select "name" where id = 1 -> "Avi"
TEST_F(SqlTest, SelectNameById)
{
	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.where("id = 1")
		>> name;
	EXPECT_EQ(name.get(), "Avi");
}

// select "name" where id = 2 -> "Batya"
TEST_F(SqlTest, SelectNameByIdSecondRow)
{
	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.where("id = 2")
		>> name;
	EXPECT_EQ(name.get(), "Batya");
}

// select "name" for all -> "Avi", "Batya"
TEST_F(SqlTest, SelectAllNames)
{
	std::vector<sql::Text> names;
	db->select<"Users", sql::SelectArg<"name">>()
		>> names;
	EXPECT_EQ(names.size(), 2);
	EXPECT_EQ(names[0].get(), "Avi");
	EXPECT_EQ(names[1].get(), "Batya");
}

// select "age" for a known row
TEST_F(SqlTest, SelectAge)
{
	sql::Real age;
	db->select<"Users", sql::SelectArg<"age">>()
		.where("id = 1")
		>> age;
	EXPECT_DOUBLE_EQ(age.get(), 22.0);
}

// select "id" (primary key)
TEST_F(SqlTest, SelectId)
{
	sql::Int id;
	db->select<"Users", sql::SelectArg<"id">>()
		.where("name = 'Batya'")
		>> id;
	EXPECT_EQ(id.get(), 2);
}

// ---------------------------------------------------------------------------
// select + where + operator>> (tuple output)
// ---------------------------------------------------------------------------

// select multiple columns into a tuple
TEST_F(SqlTest, SelectMultipleColumnsIntoTuple)
{
	using Row = std::tuple<sql::Int, sql::Text>;
	Row row;
	db->select<"Users",
		sql::SelectArg<"id">,
		sql::SelectArg<"name">>()
		.where("id = 1")
		>> row;
	EXPECT_EQ(std::get<0>(row).get(), 1);
	EXPECT_EQ(std::get<1>(row).get(), "Avi");
}

TEST_F(SqlTest, SelectAllColumnsIntoTuple)
{
	using Row = std::tuple<sql::Int, sql::Text, sql::Real, sql::Nullable<sql::Blob>>;
	Row row;
	db->select<"Users",
		sql::SelectArg<"id">,
		sql::SelectArg<"name">,
		sql::SelectArg<"age">,
		sql::SelectArg<"data">>()
		.where("id = 2")
		>> row;
	EXPECT_EQ(std::get<0>(row).get(), 2);
	EXPECT_EQ(std::get<1>(row).get(), "Batya");
	EXPECT_DOUBLE_EQ(std::get<2>(row).get(), 18.5);
	EXPECT_TRUE(std::get<3>(row).has_value());
	const auto& blob = std::get<3>(row).get().value().get();
	EXPECT_EQ(blob.size(), 3);
	EXPECT_EQ(blob[0], 0xAA);
	EXPECT_EQ(blob[1], 0xBB);
	EXPECT_EQ(blob[2], 0xCC);
}

// select multiple columns into a vector of tuples
TEST_F(SqlTest, SelectMultipleColumnsIntoTuples)
{
	using Row = std::tuple<sql::Int, sql::Text>;
	std::vector<Row> rows;
	db->select<"Users",
		sql::SelectArg<"id">,
		sql::SelectArg<"name">>()
		>> rows;
	EXPECT_EQ(rows.size(), 2);
	EXPECT_EQ(std::get<0>(rows[0]).get(), 1);
	EXPECT_EQ(std::get<1>(rows[0]).get(), "Avi");
	EXPECT_EQ(std::get<0>(rows[1]).get(), 2);
	EXPECT_EQ(std::get<1>(rows[1]).get(), "Batya");
}

TEST_F(SqlTest, SelectAllColumnsIntoTuples)
{
	using Row = std::tuple<sql::Int, sql::Text, sql::Real, sql::Nullable<sql::Blob>>;
	std::vector<Row> rows;
	db->select<"Users",
		sql::SelectArg<"id">,
		sql::SelectArg<"name">,
		sql::SelectArg<"age">,
		sql::SelectArg<"data">>()
		>> rows;
	EXPECT_EQ(rows.size(), 2);
	EXPECT_EQ(std::get<0>(rows[0]).get(), 1);
	EXPECT_EQ(std::get<1>(rows[0]).get(), "Avi");
	EXPECT_EQ(std::get<0>(rows[1]).get(), 2);
	EXPECT_DOUBLE_EQ(std::get<2>(rows[0]).get(), 22.0);
	EXPECT_FALSE(std::get<3>(rows[0]).has_value());
	EXPECT_EQ(std::get<1>(rows[1]).get(), "Batya");
	EXPECT_DOUBLE_EQ(std::get<2>(rows[1]).get(), 18.5);
	EXPECT_TRUE(std::get<3>(rows[1]).has_value());
	const auto& blob = std::get<3>(rows[1]).get().value().get();
	EXPECT_EQ(blob.size(), 3);
	EXPECT_EQ(blob[0], 0xAA);
	EXPECT_EQ(blob[1], 0xBB);
	EXPECT_EQ(blob[2], 0xCC);
}

// ---------------------------------------------------------------------------
// Nullable column
// ---------------------------------------------------------------------------

// data is NULL for row 1
TEST_F(SqlTest, NullableColumnIsNull)
{
	sql::Nullable<sql::Blob> data;
	db->select<"Users", sql::SelectArg<"data">>()
		.where("id = 1")
		>> data;
	EXPECT_FALSE(data.has_value());
	EXPECT_TRUE(data.is_null());
}

// data is not NULL for row 2
TEST_F(SqlTest, NullableColumnHasValue)
{
	sql::Nullable<sql::Blob> data;
	db->select<"Users", sql::SelectArg<"data">>()
		.where("id = 2")
		>> data;
	EXPECT_TRUE(data.has_value());
	EXPECT_FALSE(data.is_null());
}

// Dereference non-null nullable to inspect blob bytes
TEST_F(SqlTest, NullableBlobContents)
{
	sql::Nullable<sql::Blob> data;
	db->select<"Users", sql::SelectArg<"data">>()
		.where("id = 2")
		>> data;
	EXPECT_TRUE(data.has_value());
	const auto& blob = data->get();
	EXPECT_EQ(blob.size(), 3);
	EXPECT_EQ(blob[0], 0xAA);
	EXPECT_EQ(blob[1], 0xBB);
	EXPECT_EQ(blob[2], 0xCC);
}

// ---------------------------------------------------------------------------
// Callback (operator>> with callable)
// ---------------------------------------------------------------------------

// Collect all names via callback
TEST_F(SqlTest, SelectAllRowsViaCallback)
{
	std::vector<std::string> names;
	db->select<"Users", sql::SelectArg<"name">>()
		>> [&names](sql::TextView name)
		{
			names.push_back(std::string(name.get()));
		};
	EXPECT_EQ(names.size(), 2);
	// insertion order is preserved (SQLite default)
	EXPECT_EQ(names[0], "Avi");
	EXPECT_EQ(names[1], "Batya");
}

// collect multiple columns via callback
TEST_F(SqlTest, SelectMultipleColumnsViaCallback)
{
	std::vector<std::pair<std::int64_t, std::string>> rows;
	db->select<"Users",
		sql::SelectArg<"id">,
		sql::SelectArg<"name">>()
		>> [&rows](sql::IntView id, sql::TextView name)
		{
			rows.emplace_back(id.get(), std::string(name.get()));
		};
	EXPECT_EQ(rows.size(), 2);
	EXPECT_EQ(rows[0].first, 1);
	EXPECT_EQ(rows[0].second, "Avi");
	EXPECT_EQ(rows[1].first, 2);
	EXPECT_EQ(rows[1].second, "Batya");
}

// where clause narrows callback results
TEST_F(SqlTest, WhereNarrowsCallbackResults)
{
	int count = 0;
	db->select<"Users", sql::SelectArg<"id">>()
		.where("age > 20.0")
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 1); // only Avi (22.0)
}

// ---------------------------------------------------------------------------
// where clause edge cases
// ---------------------------------------------------------------------------

// where with no matching rows - callback should not fire
TEST_F(SqlTest, WhereNoMatchingRows)
{
	bool called = false;
	db->select<"Users", sql::SelectArg<"name">>()
		.where("id = 999")
		>> [&called](sql::TextView) { called = true; };
	EXPECT_FALSE(called);
}

// where + all rows match
TEST_F(SqlTest, WhereAllRowsMatch)
{
	std::vector<std::int64_t> ids;
	db->select<"Users", sql::SelectArg<"id">>()
		.where("age > 0")
		>> [&ids](sql::IntView id) { ids.push_back(id.get()); };
	EXPECT_EQ(ids.size(), 2);
}

// ---------------------------------------------------------------------------
// Column aliases (SelectArg with "as")
// ---------------------------------------------------------------------------

TEST_F(SqlTest, SelectWithAlias)
{
	sql::Text alias;
	db->select<"Users", sql::SelectArg<"name", "username">>()
		.where("id = 1")
		>> alias;
	EXPECT_EQ(alias.get(), "Avi");
}

TEST_F(SqlTest, SelectMultipleColumnsWithAlias)
{
	using Row = std::tuple<sql::Int, sql::Text>;
	Row row;
	db->select<"Users",
		sql::SelectArg<"id",   "user_id">,
		sql::SelectArg<"name", "username">>()
		.where("id = 2")
		>> row;
	EXPECT_EQ(std::get<0>(row).get(), 2);
	EXPECT_EQ(std::get<1>(row).get(), "Batya");
}

// ---------------------------------------------------------------------------
// Aggregate functions
// ---------------------------------------------------------------------------

// COUNT("id") over all rows
TEST_F(SqlTest, AggregateCount)
{
	sql::Int count;
	db->select<"Users",
		sql::AggrSelectArg<sql::Count<"id">>>()
		>> count;
	EXPECT_EQ(count.get(), 2);
}

// AVG("age") over all rows = (22.0 + 18.5) / 2 = 20.25
TEST_F(SqlTest, AggregateAvg)
{
	sql::Real avg;
	db->select<"Users",
		sql::AggrSelectArg<sql::Avg<"age">>>()
		>> avg;
	EXPECT_DOUBLE_EQ(avg.get(), 20.25);
}

// COUNT with alias
TEST_F(SqlTest, AggregateCountWithAlias)
{
	sql::Int count;
	db->select<"Users",
		sql::AggrSelectArg<sql::Count<"id">, "total">>()
		>> count;
	EXPECT_EQ(count.get(), 2);
}

// COUNT after a where clause
TEST_F(SqlTest, AggregateCountWithWhere)
{
	sql::Int count;
	db->select<"Users",
		sql::AggrSelectArg<sql::Count<"id">>>()
		.where("age >= 20.0")
		>> count;
	EXPECT_EQ(count.get(), 1); // only Avi
}

// AVG over a single remaining row after where
TEST_F(SqlTest, AggregateAvgWithWhere)
{
	sql::Real avg;
	db->select<"Users",
		sql::AggrSelectArg<sql::Avg<"age">>>()
		.where("id = 2")
		>> avg;
	EXPECT_DOUBLE_EQ(avg.get(), 18.5);
}

// ---------------------------------------------------------------------------
// Chained selects (select on TableView)
// ---------------------------------------------------------------------------

TEST_F(SqlTest, ChainedSelectNarrowsColumns)
{
	sql::Text name;
	db->select<"Users",
		sql::SelectArg<"id">,
		sql::SelectArg<"name">,
		sql::SelectArg<"age">>()
		.select<sql::SelectArg<"name">>()
		.where("id = 1")
		>> name;
	EXPECT_EQ(name.get(), "Avi");
}

// ---------------------------------------------------------------------------
// Insert + re-select (round-trip integrity)
// ---------------------------------------------------------------------------

TEST_F(SqlTest, InsertAndSelectRoundTrip)
{
	db->insert<"Users">(3, "Gal", 30.5, std::nullopt);

	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.where("id = 3")
		>> name;
	EXPECT_EQ(name.get(), "Gal");

	db->remove<"Users">("id = 3");
}

TEST_F(SqlTest, InsertWithBlobAndSelectRoundTrip)
{
	db->insert<"Users">(4, "Dani", 25.0, senc::utils::Buffer{ 0x01, 0x02 });

	sql::Nullable<sql::Blob> data;
	db->select<"Users", sql::SelectArg<"data">>()
		.where("id = 4")
		>> data;
	EXPECT_TRUE(data.has_value());
	const auto& blob = data->get();
	EXPECT_EQ(blob.size(), 2);
	EXPECT_EQ(blob[0], 0x01);
	EXPECT_EQ(blob[1], 0x02);

	db->remove<"Users">("id = 4");
}

TEST_F(SqlTest, InsertIncreasesCount)
{
	sql::Int before;
	db->select<"Users", sql::AggrSelectArg<sql::Count<"id">>>() >> before;

	db->insert<"Users">(5, "Hadas", 19.0, std::nullopt);

	sql::Int after;
	db->select<"Users", sql::AggrSelectArg<sql::Count<"id">>>() >> after;
	EXPECT_EQ(after.get(), before.get() + 1);

	db->remove<"Users">("id = 5");
	db->select<"Users", sql::AggrSelectArg<sql::Count<"id">>>() >> after;
	EXPECT_EQ(after.get(), before.get());
}

// ---------------------------------------------------------------------------
// Type conversion / operator casts
// ---------------------------------------------------------------------------

// Int is convertible to int64_t via implicit cast
TEST_F(SqlTest, IntImplicitConversion)
{
	sql::Int id;
	db->select<"Users", sql::SelectArg<"id">>()
		.where("id = 1")
		>> id;
	std::int64_t raw = id;
	EXPECT_EQ(raw, 1);
}

// Real is convertible to double via implicit cast
TEST_F(SqlTest, RealImplicitConversion)
{
	sql::Real age;
	db->select<"Users", sql::SelectArg<"age">>()
		.where("id = 1")
		>> age;
	double raw = age;
	EXPECT_DOUBLE_EQ(raw, 22.0);
}

// Text is convertible to std::string via implicit cast
TEST_F(SqlTest, TextImplicitConversion)
{
	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.where("id = 2")
		>> name;
	const std::string& raw = name;
	EXPECT_EQ(raw, "Batya");
}

// ---------------------------------------------------------------------------
// order_by
// ---------------------------------------------------------------------------

// ascending by age: Batya (18.5) before Avi (22.0)
TEST_F(SqlTest, OrderByAgeAsc)
{
	std::vector<std::string> names;
	db->select<"Users", sql::SelectArg<"name">>()
		.order_by<sql::OrderArg<"age", sql::Order::Asc>>()
		>> [&names](sql::TextView name)
		{
			names.push_back(std::string(name.get()));
		};
	EXPECT_EQ(names.size(), 2);
	EXPECT_EQ(names[0], "Batya");
	EXPECT_EQ(names[1], "Avi");
}

// descending by age: Avi (22.0) before Batya (18.5)
TEST_F(SqlTest, OrderByAgeDesc)
{
	std::vector<std::string> names;
	db->select<"Users", sql::SelectArg<"name">>()
		.order_by<sql::OrderArg<"age", sql::Order::Desc>>()
		>> [&names](sql::TextView name)
		{
			names.push_back(std::string(name.get()));
		};
	EXPECT_EQ(names.size(), 2);
	EXPECT_EQ(names[0], "Avi");
	EXPECT_EQ(names[1], "Batya");
}

// ascending by id: natural insertion order (1, 2)
TEST_F(SqlTest, OrderByIdAsc)
{
	std::vector<std::int64_t> ids;
	db->select<"Users", sql::SelectArg<"id">>()
		.order_by<sql::OrderArg<"id", sql::Order::Asc>>()
		>> [&ids](sql::IntView id) { ids.push_back(id.get()); };
	EXPECT_EQ(ids.size(), 2);
	EXPECT_EQ(ids[0], 1);
	EXPECT_EQ(ids[1], 2);
}

// descending by id: reversed order (2, 1)
TEST_F(SqlTest, OrderByIdDesc)
{
	std::vector<std::int64_t> ids;
	db->select<"Users", sql::SelectArg<"id">>()
		.order_by<sql::OrderArg<"id", sql::Order::Desc>>()
		>> [&ids](sql::IntView id) { ids.push_back(id.get()); };
	EXPECT_EQ(ids.size(), 2);
	EXPECT_EQ(ids[0], 2);
	EXPECT_EQ(ids[1], 1);
}

// order_by combined with where - only one row remains, order has no effect on count
TEST_F(SqlTest, OrderByWithWhere)
{
	std::vector<std::int64_t> ids;
	db->select<"Users", sql::SelectArg<"id">>()
		.where("age > 20.0")
		.order_by<sql::OrderArg<"age", sql::Order::Desc>>()
		>> [&ids](sql::IntView id) { ids.push_back(id.get()); };
	EXPECT_EQ(ids.size(), 1);
	EXPECT_EQ(ids[0], 1); // only Avi
}

// multiple order_by calls: primary sort by age asc, secondary by id desc
// (both rows have distinct ages so secondary key is not exercised, but the
//  chain must compile and produce the correct primary ordering)
TEST_F(SqlTest, OrderByMultipleKeys)
{
	std::vector<std::string> names;
	db->select<"Users", sql::SelectArg<"name">>()
		.order_by<sql::OrderArg<"age", sql::Order::Asc>>()
		.order_by<sql::OrderArg<"id",  sql::Order::Desc>>()
		>> [&names](sql::TextView name)
		{
			names.push_back(std::string(name.get()));
		};
	EXPECT_EQ(names.size(), 2);
	EXPECT_EQ(names[0], "Batya"); // age 18.5
	EXPECT_EQ(names[1], "Avi");   // age 22.0
}

// ---------------------------------------------------------------------------
// limit
// ---------------------------------------------------------------------------

// limit(1) returns exactly one row
TEST_F(SqlTest, LimitOne)
{
	int count = 0;
	db->select<"Users", sql::SelectArg<"id">>()
		.limit(1)
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 1);
}

// limit(2) returns both rows (no truncation)
TEST_F(SqlTest, LimitEqualsRowCount)
{
	int count = 0;
	db->select<"Users", sql::SelectArg<"id">>()
		.limit(2)
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 2);
}

// limit larger than row count returns all rows
TEST_F(SqlTest, LimitExceedsRowCount)
{
	int count = 0;
	db->select<"Users", sql::SelectArg<"id">>()
		.limit(100)
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 2);
}

// limit(0) returns no rows
TEST_F(SqlTest, LimitZero)
{
	bool called = false;
	db->select<"Users", sql::SelectArg<"id">>()
		.limit(0)
		>> [&called](sql::IntView) { called = true; };
	EXPECT_FALSE(called);
}

// limit combined with order_by: first row in descending age order is Avi
TEST_F(SqlTest, LimitWithOrderBy)
{
	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.order_by<sql::OrderArg<"age", sql::Order::Desc>>()
		.limit(1)
		>> name;
	EXPECT_EQ(name.get(), "Avi");
}

// limit combined with where
TEST_F(SqlTest, LimitWithWhere)
{
	int count = 0;
	db->select<"Users", sql::SelectArg<"id">>()
		.where("age > 0")
		.limit(1)
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 1);
}

// ---------------------------------------------------------------------------
// offset
// ---------------------------------------------------------------------------

// offset(0) is a no-op: both rows still returned
TEST_F(SqlTest, OffsetZero)
{
	int count = 0;
	db->select<"Users", sql::SelectArg<"id">>()
		.offset(0)
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 2);
}

// offset(1) skips the first row, leaving one
TEST_F(SqlTest, OffsetOne)
{
	int count = 0;
	db->select<"Users", sql::SelectArg<"id">>()
		.offset(1)
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 1);
}

// offset beyond row count yields no results
TEST_F(SqlTest, OffsetBeyondRowCount)
{
	bool called = false;
	db->select<"Users", sql::SelectArg<"id">>()
		.offset(100)
		>> [&called](sql::IntView) { called = true; };
	EXPECT_FALSE(called);
}

// offset(1) with order_by asc: skips Batya (18.5), delivers Avi (22.0)
TEST_F(SqlTest, OffsetWithOrderBy)
{
	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.order_by<sql::OrderArg<"age", sql::Order::Asc>>()
		.offset(1)
		>> name;
	EXPECT_EQ(name.get(), "Avi");
}

// ---------------------------------------------------------------------------
// limit + offset combined
// ---------------------------------------------------------------------------

// limit(1) + offset(0): first row in default order -> Avi
TEST_F(SqlTest, LimitOneOffsetZero)
{
	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.order_by<sql::OrderArg<"id", sql::Order::Asc>>()
		.limit(1)
		.offset(0)
		>> name;
	EXPECT_EQ(name.get(), "Avi");
}

// limit(1) + offset(1): second row in ascending id order -> Batya
TEST_F(SqlTest, LimitOneOffsetOne)
{
	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.order_by<sql::OrderArg<"id", sql::Order::Asc>>()
		.limit(1)
		.offset(1)
		>> name;
	EXPECT_EQ(name.get(), "Batya");
}

// limit(1) + offset(1) + where that matches both rows: same second-row result
TEST_F(SqlTest, LimitOffsetWithWhere)
{
	sql::Text name;
	db->select<"Users", sql::SelectArg<"name">>()
		.where("age > 0")
		.order_by<sql::OrderArg<"id", sql::Order::Asc>>()
		.limit(1)
		.offset(1)
		>> name;
	EXPECT_EQ(name.get(), "Batya");
}

// offset past the limited window yields no rows
TEST_F(SqlTest, LimitOffsetBothBeyondRows)
{
	bool called = false;
	db->select<"Users", sql::SelectArg<"id">>()
		.limit(1)
		.offset(2)
		>> [&called](sql::IntView) { called = true; };
	EXPECT_FALSE(called);
}

// ---------------------------------------------------------------------------
// join
// ---------------------------------------------------------------------------

// join produces one row per matching pair: 2 Users x 1 FavNumber each = 2 rows
TEST_F(SqlTest, JoinRowCount)
{
	int count = 0;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 2);
}

// Avi's favourite number is 434
TEST_F(SqlTest, JoinSelectFavNumForAvi)
{
	sql::Int fav;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		.where("name = 'Avi'")
		>> fav;
	EXPECT_EQ(fav.get(), 434);
}

// Batya's favourite number is 256
TEST_F(SqlTest, JoinSelectFavNumForBatya)
{
	sql::Int fav;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		.where("name = 'Batya'")
		>> fav;
	EXPECT_EQ(fav.get(), 256);
}

// select both name and fav_num via callback, verify each pair
TEST_F(SqlTest, JoinSelectNameAndFavNumViaCallback)
{
	std::vector<std::pair<std::string, std::int64_t>> rows;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"name">, sql::SelectArg<"fav_num">>()
		.order_by<sql::OrderArg<"id", sql::Order::Asc>>()
		>> [&rows](sql::TextView name, sql::IntView fav)
		{
			rows.emplace_back(std::string(name.get()), fav.get());
		};
	EXPECT_EQ(rows.size(), 2);
	EXPECT_EQ(rows[0].first,  "Avi");   EXPECT_EQ(rows[0].second, 434);
	EXPECT_EQ(rows[1].first,  "Batya"); EXPECT_EQ(rows[1].second, 256);
}

// join + where filters down to one pair
TEST_F(SqlTest, JoinWhereFiltersToOneRow)
{
	int count = 0;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		.where("fav_num > 300")
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 1); // only Avi (434)
}

// where with no matching join rows - callback should not fire
TEST_F(SqlTest, JoinWhereNoMatchingRows)
{
	bool called = false;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		.where("fav_num = 999")
		>> [&called](sql::IntView) { called = true; };
	EXPECT_FALSE(called);
}

// join + order_by fav_num asc: 256 (Batya) before 434 (Avi)
TEST_F(SqlTest, JoinOrderByFavNumAsc)
{
	std::vector<std::int64_t> favs;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		.order_by<sql::OrderArg<"fav_num", sql::Order::Asc>>()
		>> [&favs](sql::IntView fav) { favs.push_back(fav.get()); };
	EXPECT_EQ(favs.size(), 2);
	EXPECT_EQ(favs[0], 256);
	EXPECT_EQ(favs[1], 434);
}

// join + order_by fav_num desc: 434 (Avi) before 256 (Batya)
TEST_F(SqlTest, JoinOrderByFavNumDesc)
{
	std::vector<std::int64_t> favs;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		.order_by<sql::OrderArg<"fav_num", sql::Order::Desc>>()
		>> [&favs](sql::IntView fav) { favs.push_back(fav.get()); };
	EXPECT_EQ(favs.size(), 2);
	EXPECT_EQ(favs[0], 434);
	EXPECT_EQ(favs[1], 256);
}

// join + limit(1) returns exactly one row
TEST_F(SqlTest, JoinLimit)
{
	int count = 0;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		.limit(1)
		>> [&count](sql::IntView) { ++count; };
	EXPECT_EQ(count, 1);
}

// join + limit(1) + offset(1) skips first row, delivers second
TEST_F(SqlTest, JoinLimitOffset)
{
	sql::Int fav;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::SelectArg<"fav_num">>()
		.order_by<sql::OrderArg<"fav_num", sql::Order::Asc>>()
		.limit(1)
		.offset(1)
		>> fav;
	EXPECT_EQ(fav.get(), 434); // second in asc order
}

// join + aggregate: COUNT over the joined rows
TEST_F(SqlTest, JoinAggregateCount)
{
	sql::Int count;
	db->join<"Users", "id", "FavNumbers", "user_id">()
		.select<sql::AggrSelectArg<sql::Count<"fav_num">>>()
		>> count;
	EXPECT_EQ(count.get(), 2);
}

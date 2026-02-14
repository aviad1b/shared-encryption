/*********************************************************************
 * \file   test_sql.cpp
 * \brief  Contains tests for SQL-related utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Shvat 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <optional>
#include <tuple>

#include "../utils/sqlite/Database.hpp"

namespace sql = senc::utils::sqlite;

class SqlTest : public testing::Test
{
protected:
	// schema: Users(id: INT PK, name: TEXT, age: REAL, data: BLOB?)
	std::optional<sql::TempDatabase<sql::schemas::DB<
		sql::schemas::Table<"Users",
			sql::schemas::PrimaryKey<"id"  , sql::Int                >,
			sql::schemas::Col       <"name", sql::Text               >,
			sql::schemas::Col       <"age" , sql::Real               >,
			sql::schemas::Col       <"data", sql::Nullable<sql::Blob>>
		>
	>>> db;

	void SetUp() override
	{
		db.emplace("database.sqlite");

		db->insert<"Users">(1, "Avi",   22.0,  std::nullopt);
		db->insert<"Users">(2, "Batya", 18.5,  senc::utils::Buffer{ 0xAA, 0xBB, 0xCC });
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
	ASSERT_TRUE(std::get<3>(row).has_value());
	const auto& blob = std::get<3>(row).get().value().get();
	ASSERT_EQ(blob.size(), 3u);
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
	ASSERT_TRUE(data.has_value());
	EXPECT_FALSE(data.is_null());
}

// Dereference non-null nullable to inspect blob bytes
TEST_F(SqlTest, NullableBlobContents)
{
	sql::Nullable<sql::Blob> data;
	db->select<"Users", sql::SelectArg<"data">>()
		.where("id = 2")
		>> data;
	ASSERT_TRUE(data.has_value());
	const auto& blob = data->get();
	ASSERT_EQ(blob.size(), 3u);
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
	ASSERT_EQ(names.size(), 2u);
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
	ASSERT_EQ(rows.size(), 2u);
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
	EXPECT_EQ(ids.size(), 2u);
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

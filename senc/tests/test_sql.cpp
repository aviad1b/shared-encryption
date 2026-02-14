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

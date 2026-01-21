/*********************************************************************
 * \file   test_bytes.cpp
 * \brief  Contains tests for byte-related utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#include <gtest/gtest.h>

#include "../utils/bytes.hpp"

using senc::utils::write_bytes;
using senc::utils::read_bytes;
using senc::utils::Buffer;

TEST(BytesTests, BufferWriteRead)
{
	enum class MyEnum { A, B, C };

	const std::string outStr = "abc";
	const std::wstring outWstr = L"def";
	const int outNum = 5;
	MyEnum outEnum = MyEnum::B;
	Buffer outSubBuf{ 1, 2, 3 };

	Buffer buff{};
	write_bytes(buff, outStr);
	write_bytes(buff, outWstr);
	write_bytes(buff, outNum);
	write_bytes(buff, outEnum);
	write_bytes(buff, outSubBuf);

	std::string inStr{};
	std::wstring inWstr{};
	int inNum{};
	MyEnum inEnum{};
	Buffer inSubBuf(outSubBuf.size(), 0);

	const auto end = buff.end();
	auto it = buff.begin();

	it = read_bytes(inStr, it, end);
	EXPECT_EQ(inStr, outStr);
	EXPECT_NE(it, end);

	it = read_bytes(inWstr, it, end);
	EXPECT_EQ(inWstr, outWstr);
	EXPECT_NE(it, end);

	it = read_bytes(inNum, it, end);
	EXPECT_EQ(inNum, outNum);
	EXPECT_NE(it, end);

	it = read_bytes(inEnum, it, end);
	EXPECT_EQ(inEnum, outEnum);
	EXPECT_NE(it, end);

	it = read_bytes(inSubBuf, it, end);
	EXPECT_EQ(inSubBuf, outSubBuf);
	EXPECT_EQ(it, end);
}

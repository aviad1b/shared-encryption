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

template <typename Self>
concept EndianessWrapper = requires
{
	{ Self::value } -> std::convertible_to<std::endian>;
};

template <EndianessWrapper E>
struct BytesTests : public testing::Test { };

using EndianessTypes = testing::Types<
	std::integral_constant<std::endian, std::endian::big>,
	std::integral_constant<std::endian, std::endian::little>
>;
TYPED_TEST_SUITE(BytesTests, EndianessTypes);

TYPED_TEST(BytesTests, BufferWriteRead)
{
	constexpr std::endian endianess = TypeParam::value;

	enum class MyEnum { A, B, C };

	const std::string outStr = "abc";
	const std::wstring outWstr = L"def";
	const int outNum = 5;
	MyEnum outEnum = MyEnum::B;
	Buffer outSubBuf{ 1, 2, 3 };

	Buffer buff{};
	write_bytes<endianess>(buff, outStr);
	write_bytes<endianess>(buff, outWstr);
	write_bytes<endianess>(buff, outNum);
	write_bytes<endianess>(buff, outEnum);
	write_bytes<endianess>(buff, outSubBuf);

	std::string inStr{};
	std::wstring inWstr{};
	int inNum{};
	MyEnum inEnum{};
	Buffer inSubBuf(outSubBuf.size(), 0);

	const auto end = buff.end();
	auto it = buff.begin();

	it = read_bytes<endianess>(inStr, it, end);
	EXPECT_EQ(inStr, outStr);
	EXPECT_NE(it, end);

	it = read_bytes<endianess>(inWstr, it, end);
	EXPECT_EQ(inWstr, outWstr);
	EXPECT_NE(it, end);

	it = read_bytes<endianess>(inNum, it, end);
	EXPECT_EQ(inNum, outNum);
	EXPECT_NE(it, end);

	it = read_bytes<endianess>(inEnum, it, end);
	EXPECT_EQ(inEnum, outEnum);
	EXPECT_NE(it, end);

	it = read_bytes<endianess>(inSubBuf, it, end);
	EXPECT_EQ(inSubBuf, outSubBuf);
	EXPECT_EQ(it, end);
}

/*********************************************************************
 * \file   test_bin_file.cpp
 * \brief  Contains tests for the `utils::BinFile` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include <gtest/gtest.h>
#include <filesystem>
#include <cstdint>
#include "../utils/BinFile.hpp"

namespace fs = std::filesystem;
using senc::utils::AccessFlags;
using senc::utils::BinFile;
using senc::utils::byte;

// helper to create a temp file path
static std::string tmp_path(const std::string& name)
{
    return (fs::temp_directory_path() / name).string();
}

// helper to write raw bytes to a file for test setup
static void write_raw(const std::string& path, const std::vector<byte>& data)
{
    std::FILE* f = std::fopen(path.c_str(), "wb");
    std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
}

// helper to convert value to bytes based on endianess
template <std::integral T>
static std::vector<byte> to_bytes(T value, std::endian endianess)
{
    std::vector<byte> bytes(sizeof(T));
    std::memcpy(bytes.data(), &value, sizeof(T));
    if (endianess != std::endian::native)
        std::reverse(bytes.begin(), bytes.end());
    return bytes;
}

static std::vector<byte> read_raw(const std::string& path)
{
    std::FILE* f = std::fopen(path.c_str(), "rb");
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::rewind(f);
    std::vector<byte> buf(size);
    std::fread(buf.data(), 1, size, f);
    std::fclose(f);
    return buf;
}

template <typename Self>
concept EndianessWrapper = requires
{
    { Self::value } -> std::convertible_to<std::endian>;
};

template <EndianessWrapper E>
struct BinFileTest : public ::testing::Test
{
protected:
    std::string path;

    void SetUp() override
    {
        path = tmp_path("binfile_test.bin");
        // Ensure a clean file exists
        write_raw(path, {});
    }

    void TearDown() override
    {
        fs::remove(path);
    }
};

using EndianessTypes = testing::Types<
    std::integral_constant<std::endian, std::endian::big>,
    std::integral_constant<std::endian, std::endian::little>
>;
TYPED_TEST_SUITE(BinFileTest, EndianessTypes);

// size() tests

TYPED_TEST(BinFileTest, SizeOfEmptyFile)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    BinFile<AccessFlags::Read, endianess> f(path);
    EXPECT_EQ(f.size(), 0);
}

TYPED_TEST(BinFileTest, SizeMatchesWrittenBytes)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0x01, 0x02, 0x03});
    BinFile<AccessFlags::Read, endianess> f(path);
    EXPECT_EQ(f.size(), 3);
}

TYPED_TEST(BinFileTest, SizeAfterWriting)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    BinFile<AccessFlags::Write, endianess> f(path);
    EXPECT_EQ(f.size(), 0);
    f.template write<byte>(0xAA);
    EXPECT_EQ(f.size(), 1);
    f.template write<std::uint32_t>(0x12345678);
    EXPECT_EQ(f.size(), 5);
}

TYPED_TEST(BinFileTest, SizeIsIndependentOfPosition)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {1, 2, 3, 4, 5});
    BinFile<AccessFlags::Read, endianess> f(path);
    f.set_pos(3);
    EXPECT_EQ(f.size(), 5); // size shouldn't change as we seek around
    f.template read<byte>();
    EXPECT_EQ(f.size(), 5);
}

TYPED_TEST(BinFileTest, SizeAfterAppend)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0x01, 0x02});
    BinFile<AccessFlags::Append, endianess> f(path);
    EXPECT_EQ(f.size(), 2);
    f.template append<byte>(0x03);
    EXPECT_EQ(f.size(), 3);
    f.template append<std::uint16_t>(0xABCD);
    EXPECT_EQ(f.size(), 5);
}

TYPED_TEST(BinFileTest, SizeAfterEditDoesNotGrow)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0xAA, 0xBB, 0xCC});
    BinFile<AccessFlags::Edit, endianess> f(path);
    EXPECT_EQ(f.size(), 3);
    f.set_pos(1);
    f.template write<byte>(0xFF); // overwrite, not append
    EXPECT_EQ(f.size(), 3);
}

// read tests

TYPED_TEST(BinFileTest, ReadSingleByte)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0xAB});
    BinFile<AccessFlags::Read, endianess> f(path);
    auto val = f.template read<byte>();
    EXPECT_EQ(val, 0xAB);
}

TYPED_TEST(BinFileTest, ReadDefaultTypeIsByte)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0x35});
    BinFile<AccessFlags::Read, endianess> f(path);
    auto val = f.read(); // default template param should be byte
    EXPECT_EQ(val, 0x35);
}

TYPED_TEST(BinFileTest, ReadMultipleBytes)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {1, 2, 3, 4, 5});
    BinFile<AccessFlags::Read, endianess> f(path);
    byte buff[5]{};
    f.read(buff, 5);
    for (std::size_t i = 0; i < 5; i++)
        EXPECT_EQ(buff[i], i + 1);
}

TYPED_TEST(BinFileTest, ReadUint32)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, to_bytes<std::uint32_t>(0x01020304, endianess));
    BinFile<AccessFlags::Read, endianess> f(path);
    auto val = f.template read<std::uint32_t>();
    EXPECT_EQ(val, 0x01020304);
}

TYPED_TEST(BinFileTest, ReadSequential)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {10, 20, 30});
    BinFile<AccessFlags::Read, endianess> f(path);
    EXPECT_EQ(f.template read<byte>(), 10);
    EXPECT_EQ(f.template read<byte>(), 20);
    EXPECT_EQ(f.template read<byte>(), 30);
}

// get_pos / set_pos tests

TYPED_TEST(BinFileTest, InitialPositionIsZero)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {1, 2, 3});
    BinFile<AccessFlags::Read, endianess> f(path);
    EXPECT_EQ(f.get_pos(), 0);
}

TYPED_TEST(BinFileTest, PosAdvancesAfterRead)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {1, 2, 3, 4});
    BinFile<AccessFlags::Read, endianess> f(path);
    f.template read<byte>();
    EXPECT_EQ(f.get_pos(), 1);
    f.template read<std::uint16_t>();
    EXPECT_EQ(f.get_pos(), 3);
}

TYPED_TEST(BinFileTest, SetPosAndRead)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0xAA, 0xBB, 0xCC});
    BinFile<AccessFlags::Read, endianess> f(path);
    f.set_pos(2);
    EXPECT_EQ(f.get_pos(), 2);
    EXPECT_EQ(f.template read<byte>(), 0xCC);
}

TYPED_TEST(BinFileTest, SetPosToBeginningAndReread)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0x11, 0x22});
    BinFile<AccessFlags::Read, endianess> f(path);
    EXPECT_EQ(f.template read<byte>(), 0x11);
    f.set_pos(0);
    EXPECT_EQ(f.template read<byte>(), 0x11);  // read same byte again
}

// write tests

TYPED_TEST(BinFileTest, WriteSingleByte)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    {
        BinFile<AccessFlags::Write, endianess> f(path);
        f.template write<byte>(0xDE);
    }
    auto data = read_raw(path);
    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], 0xDE);
}

TYPED_TEST(BinFileTest, WriteMultipleBytes)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    const byte src[] = {1, 2, 3, 4, 5};
    {
        BinFile<AccessFlags::Write, endianess> f(path);
        f.write(src, 5);
    }
    auto data = read_raw(path);
    ASSERT_EQ(data.size(), 5);
    for (int i = 0; i < 5; i++)
        EXPECT_EQ(data[i], src[i]);
}

TYPED_TEST(BinFileTest, WriteUint32)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    {
        BinFile<AccessFlags::Write, endianess> f(path);
        f.template write<std::uint32_t>(0xDEADBEEF);
    }
    auto data = read_raw(path);
    ASSERT_EQ(data.size(), 4);
    EXPECT_EQ(data, to_bytes<std::uint32_t>(0xDEADBEEF, endianess));
}

TYPED_TEST(BinFileTest, WriteAdvancesPosition)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    BinFile<AccessFlags::Write, endianess> f(path);
    f.template write<byte>(0x01);
    EXPECT_EQ(f.get_pos(), 1);
    f.template write<std::uint16_t>(0x0203);
    EXPECT_EQ(f.get_pos(), 3);
}

// edit (rb+) tests

TYPED_TEST(BinFileTest, EditReadThenWrite)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0xAA, 0xBB, 0xCC});
    {
        BinFile<AccessFlags::Edit, endianess> f(path);
        auto first = f.template read<byte>();
        EXPECT_EQ(first, 0xAA);
        f.template write<byte>(0xFF);  // overwrites 0xBB
    }
    auto data = read_raw(path);
    EXPECT_EQ(data[0], 0xAA);
    EXPECT_EQ(data[1], 0xFF);
    EXPECT_EQ(data[2], 0xCC);
}

TYPED_TEST(BinFileTest, EditSetPosAndOverwrite)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0x01, 0x02, 0x03, 0x04});
    {
        BinFile<AccessFlags::Edit, endianess> f(path);
        f.set_pos(2);
        f.template write<byte>(0xAB);
    }
    auto data = read_raw(path);
    EXPECT_EQ(data[2], 0xAB);
    EXPECT_EQ(data[0], 0x01);  // untouched
}

// append tests

TYPED_TEST(BinFileTest, AppendSingleByte)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0x01, 0x02});
    {
        BinFile<AccessFlags::Append, endianess> f(path);
        f.template append<byte>(0x03);
    }
    auto data = read_raw(path);
    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[2], 0x03);
}

TYPED_TEST(BinFileTest, AppendMultipleBytes)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    write_raw(path, {0xAA});
    const byte extra[] = {0xBB, 0xCC};
    {
        BinFile<AccessFlags::Append, endianess> f(path);
        f.append(extra, 2);
    }
    auto data = read_raw(path);
    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[0], 0xAA);
    EXPECT_EQ(data[1], 0xBB);
    EXPECT_EQ(data[2], 0xCC);
}

TYPED_TEST(BinFileTest, AppendToEmptyFile)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    {
        BinFile<AccessFlags::Append, endianess> f(path);
        f.template append<std::uint16_t>(0x1234);
    }
    auto data = read_raw(path);
    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data, to_bytes<std::uint16_t>(0x1234, endianess));
}

// read | write combined tests

TYPED_TEST(BinFileTest, ReadWriteRoundtrip)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    const byte src[] = {10, 20, 30, 40};
    {
        BinFile<AccessFlags::Read | AccessFlags::Write, endianess> f(path);
        f.write(src, 4);
        f.set_pos(0);
        byte buf[4]{};
        f.read(buf, 4);
        for (int i = 0; i < 4; i++)
            EXPECT_EQ(buf[i], src[i]);
    }
}

TYPED_TEST(BinFileTest, ReadWritePosConsistentAcrossOps)
{
    constexpr std::endian endianess = TypeParam::value;
    const auto& path = this->path;

    {
        BinFile<AccessFlags::Read | AccessFlags::Write, endianess> f(path);
        f.template write<byte>(0x11);
        f.template write<byte>(0x22);
        EXPECT_EQ(f.get_pos(), 2);
        f.set_pos(0);
        EXPECT_EQ(f.template read<byte>(), 0x11);
        EXPECT_EQ(f.get_pos(), 1);
    }
}

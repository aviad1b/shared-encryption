/*********************************************************************
 * \file   tests_utils.cpp
 * \brief  Implementation of utilities for tests.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "tests_utils.hpp"

std::string temp_file_path(const std::string& name)
{
	return (std::filesystem::temp_directory_path() / name).string();
}

void write_raw(const std::string& path, const std::vector<byte>& data)
{
	std::FILE* f = std::fopen(path.c_str(), "wb");
	std::fwrite(data.data(), 1, data.size(), f);
	std::fclose(f);
}

std::vector<byte> read_raw(const std::string& path)
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

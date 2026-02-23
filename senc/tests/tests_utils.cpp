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

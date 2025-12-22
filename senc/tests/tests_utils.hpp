/*********************************************************************
 * \file   tests_utils.hpp
 * \brief  Header of utilities for tests.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <gtest/gtest.h>
#include <tuple>
#include "../utils/Socket.hpp"
#include "../utils/hash.hpp"

/**
 * @brief Prepares local TCP connection for test.
 */
std::tuple<senc::utils::TcpSocket<senc::utils::IPv4>, senc::utils::TcpSocket<senc::utils::IPv4>> prepare_tcp();

/**
 * @brief Checks if two vectors have the same elements, regardless of order.
 */
template <typename T>
void EXPECT_SAME_ELEMS(const std::vector<T>& a, const std::vector<T>& b)
{
	senc::utils::HashSet<T> setA(a.begin(), a.end());
	senc::utils::HashSet<T> setB(b.begin(), b.end());
	EXPECT_EQ(setA, setB);
}

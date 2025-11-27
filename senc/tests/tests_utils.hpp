/*********************************************************************
 * \file   tests_utils.hpp
 * \brief  Header of utilities for tests.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <tuple>
#include "../utils/Socket.hpp"

/**
 * @brief Prepares local TCP connection for test.
 */
std::tuple<senc::utils::TcpSocket<senc::utils::IPv4>, senc::utils::TcpSocket<senc::utils::IPv4>> prepare_tcp();

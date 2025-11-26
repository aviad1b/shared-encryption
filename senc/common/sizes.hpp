/*********************************************************************
 * \file   sizes.hpp
 * \brief  Contains information about input sizes.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include <cstdint>
#include <cstdlib>
#include <limits>

namespace senc
{
	/**
	 * @typedef senc::member_count_t
	 * @brief Fundamental used for counting members in a userset.
	 */
	using member_count_t = std::uint8_t;

	/**
	 * @brief Maximum members in a userset.
	 */
	constexpr std::size_t MAX_MEMBERS = std::numeric_limits<member_count_t>::max();

	/**
	 * @typedef senc::userset_count_t
	 * @brief Fundamental used for counting usersets owned by a user.
	 */
	using userset_count_t = std::uint8_t;

	/**
	 * @brief Maximum usersets owned by a user.
	 */
	constexpr std::size_t MAX_USERSETS = std::numeric_limits<userset_count_t>::max();

	/**
	 * @typedef senc::lookup_count_t
	 * @brief Fundamental used for counting lookups for a specific user.
	 */
	using lookup_count_t = std::uint8_t;

	/**
	 * @brief Maximum lookups for an individual user.
	 */
	constexpr std::size_t MAX_LOOKUP = std::numeric_limits<lookup_count_t>::max();

	/**
	 * @typedef senc::pending_count_t
	 * @brief Fundamental used for counting pending decryptions requiring a specific user.
	 */
	using pending_count_t = std::uint8_t;

	/**
	 * @brief Maximum pending decryptions requiring an individual user.
	 */
	constexpr std::size_t MAX_PENDING = std::numeric_limits<pending_count_t>::max();

	/**
	 * @typedef senc::res_count_t
	 * @brief Fundamental used for counting operation results for a single requester.
	 */
	using res_count_t = std::uint8_t;

	/**
	 * @brief Maximum operation results for a single requester.
	 */
	constexpr std::size_t MAX_RESULTS = std::numeric_limits<res_count_t>::max();
}

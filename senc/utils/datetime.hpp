/*********************************************************************
 * \file   datetime.hpp
 * \brief  Contains datetime-related structs.
 * 
 * \author aviad1b
 * \date   January 2026, Shvat 5786
 *********************************************************************/

#pragma once

#include <cstdint>

namespace senc::utils
{
	/**
	 * @struct senc::utils::Date
	 * @brief Represents a date.
	 */
	struct Date
	{
		std::int32_t year;
		std::uint8_t month;
		std::uint8_t day;
	};

	/**
	 * @struct senc::utils::Time
	 * @brief Represents a time in the day.
	 */
	struct Time
	{
		std::uint8_t hours;
		std::uint8_t minutes;
		std::uint8_t seconds;
	};

	/**
	 * @struct senc::utils::DateTime
	 * @brief Holds together a date and a time.
	 */
	struct DateTime
	{
		Date date;
		Time time;
	};
}

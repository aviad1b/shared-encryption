/*********************************************************************
 * \file   DummyLogger.hpp
 * \brief  Contains DummyLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "Logger.hpp"

namespace senc::server
{
	class DummyLogger : public Logger
	{
	public:
		using Self = DummyLogger;
		using Base = Logger;

		void log_info(const std::string& msg) override { (void)msg; }
	};
}

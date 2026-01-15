/*********************************************************************
 * \file   DummyLogger.hpp
 * \brief  Contains DummyLogger class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "ILogger.hpp"

namespace senc::server
{
	class DummyLogger : public ILogger
	{
	public:
		using Self = DummyLogger;

		void log_info(const std::string& msg) override { (void)msg; }
		
		void log_error(const std::string& msg) override { (void)msg; }
	};
}

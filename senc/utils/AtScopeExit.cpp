/*********************************************************************
 * \file   AtScopeExit.cpp
 * \brief  Implementation of AtScopeExit class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "AtScopeExit.hpp"

namespace senc::utils
{
	AtScopeExit::AtScopeExit(std::function<void()> func) : _func(func) { }

	AtScopeExit::~AtScopeExit()
	{
		_func();
	}
}

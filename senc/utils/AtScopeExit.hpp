/*********************************************************************
 * \file   AtScopeExit.hpp
 * \brief  Header of AtScopeExit class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include <functional>

namespace senc::utils
{
	/**
	 * @class senc::utils::AtScopeExit
	 * @brief Used for calling a function at the end of a scope.
	 */
	class AtScopeExit
	{
	public:
		using Self = AtScopeExit;

		/**
		 * @brief Constructor of `AtScopeExit`, sets function to call at destruction.
		 * @param func Function to call at destruction.
		 */
		AtScopeExit(std::function<void()> func);

		/**
		 * @brief Destructor of `AtScopeExit`, calls function given at construction.
		 */
		~AtScopeExit();

	private:
		std::function<void()> _func;
	};
}

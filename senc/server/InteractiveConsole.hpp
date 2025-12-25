/*********************************************************************
 * \file   InteractiveConsole.hpp
 * \brief  Header of InteractiveConsole class.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#pragma once

#include <functional>
#include <string>

namespace senc::server
{
	/**
	 * @class senc::server::InteractiveConsole
	 * @brief Manages interactive console (with simultanious input and output).
	 */
	class InteractiveConsole
	{
	public:
		using Self = InteractiveConsole;

		/**
		 * @brief Constructs interactive console.
		 * @param handleInput A function used to handle user input.
		 */
		InteractiveConsole(std::function<void(const std::string&)> handleInput);

		/**
		 * @brief Starts interactive console.
		 */
		void start();

		/**
		 * @brief Prints message to interactive console.
		 * @param msg Message to print.
		 */
		void print(const std::string& msg);

		/**
		 * @brief Stops interactive console.
		 */
		void stop();

	private:
		std::function<void(const std::string&)> _handleInput;
	};
}

/*********************************************************************
 * \file   InteractiveConsole.hpp
 * \brief  Header of InteractiveConsole class.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#pragma once

#include <WinSock2.h> // has to be before <Windows.h> (to be able to use sockets in other files)
#include <Windows.h>

#include <functional>
#include <atomic>
#include <string>
#include <mutex>

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
		 * @param handleInput A function used to handle user input, returning `true` if should stop.
		 */
		InteractiveConsole(std::function<bool(Self&, const std::string&)> handleInput);

		/**
		 * @brief Destructor of InteractiveConsole, ensures console is stopped.
		 */
		~InteractiveConsole();

		/**
		 * @brief Starts interactive console.
		 */
		void start_inputs();

		/**
		 * @brief Prints message to interactive console.
		 * @param msg Message to print.
		 */
		void print(const std::string& msg);

		/**
		 * @brief Stops interactive console.
		 */
		void stop_inputs();

	private:
		std::function<bool(Self&, const std::string&)> _handleInput;
		std::atomic<bool> _running;
		bool _handlingInput;
		std::mutex _mtxOut;

		HANDLE _hStdin;
		HANDLE _hStdout;

		std::string _curIn;

		void input_loop();

		void handle_key_event(const KEY_EVENT_RECORD& keyEvent);

		void display_prompt();

		void clear_current_line();

		void write_to_console(const std::string& text);
	};
}

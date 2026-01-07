/*********************************************************************
 * \file   InteractiveConsole.hpp
 * \brief  Header of InteractiveConsole class.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#pragma once

#include "../utils/env.hpp"

#ifdef SENC_WINDOWS
#include "../utils/winapi_patch.hpp"
#else
#include <termios.h>
#endif

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

#ifdef SENC_WINDOWS
		HANDLE _hStdin;
		HANDLE _hStdout;
#else
		int _stdin_fd;
		int _stdout_fd;
		struct termios _original_termios;
#endif

		std::string _curIn;

		/**
		 * @brief Reads inputs from user until console is stopped
		 *        (via `stop_input` or a `true` return from `handleInput`).
		 */
		void input_loop();

#ifdef SENC_WINDOWS
		/**
		 * @brief Handles console key event.
		 * @param keyEvent Key event params.
		 */
		void handle_key_event(const KEY_EVENT_RECORD& keyEvent);
#else
		/**
		 * @brief Handles console key event.
		 * @param ch Character representing pressed key.
		 */
		void handle_key_event(char ch);
#endif

		/**
		 * @brief Displays input prompt.
		 */
		void display_prompt();

		/**
		 * @brief Clears current console line.
		 */
		void clear_current_line();

		/**
		 * @brief Writes text directly to console.
		 * @param text Text to write to console.
		 */
		void write_to_console(const std::string& text);
	};
}

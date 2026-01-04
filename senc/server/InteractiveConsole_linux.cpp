/*********************************************************************
 * \file   InteractiveConsole_linux.cpp
 * \brief  Implementation of InteractiveConsole class for Linux.
 *
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#include "InteractiveConsole.hpp"

#ifndef SENC_WINDOWS

#include "../utils/Exception.hpp"

#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace senc::server
{
	InteractiveConsole::InteractiveConsole(std::function<bool(Self&, const std::string&)> handleInput)
		: _handleInput(handleInput),
		 _running(false),
		 _handlingInput(false),
		 _stdin_fd(STDIN_FILENO),
		 _stdout_fd(STDOUT_FILENO)
	{
		if (!isatty(_stdin_fd) || !isatty(_stdout_fd))
			throw utils::Exception("Failed to initialize console: not a TTY");

		// save original terminal settings
		if (tcgetattr(_stdin_fd, &_original_termios) < 0)
			throw utils::Exception("Failed to get terminal attributes");
	}

	InteractiveConsole::~InteractiveConsole()
	{
		stop_inputs();
	}

	void InteractiveConsole::start_inputs()
	{
		if (_running.exchange(true))
			return; // already running

		// set terminal to raw mode
		struct termios raw = _original_termios;
		raw.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo
		raw.c_cc[VMIN] = 0;  // non-blocking read
		raw.c_cc[VTIME] = 0; // no timeout

		if (tcsetattr(_stdin_fd, TCSAFLUSH, &raw) < 0)
			throw utils::Exception("Failed to set raw mode");

		// display initial prompt
		display_prompt();

		// run input loop on this thread
		input_loop();

		// restore terminal settings when done
		tcsetattr(_stdin_fd, TCSAFLUSH, &_original_termios);
	}

	void InteractiveConsole::print(const std::string& msg)
	{
		// if input loop is not running or mid-handling input, do regular print
		if (!_running || _handlingInput)
			write_to_console(msg + "\n");
		else
		{
			const std::lock_guard<std::mutex> lock(_mtxOut);

			// clear and print
			clear_current_line();
			write_to_console(msg + "\n");

			// redraw prompt and current input
			display_prompt();
			write_to_console(_curIn);
		}
	}

	void InteractiveConsole::stop_inputs()
	{
		_running = false;
	}

	void InteractiveConsole::input_loop()
	{
		fd_set readfds;
		struct timeval tv;
		char ch;

		while (_running)
		{
			// set up select with timeout to check _running periodically
			FD_ZERO(&readfds);
			FD_SET(_stdin_fd, &readfds);

			tv.tv_sec = 0;
			tv.tv_usec = 100000; // 100ms timeout

			int ret = select(_stdin_fd + 1, &readfds, nullptr, nullptr, &tv);

			if (ret < 0)
				break; // error

			if (ret == 0)
				continue; // timeout, check _running again

			// read one character
			ssize_t n = read(_stdin_fd, &ch, 1);
			if (n <= 0)
				continue;

			handle_key_event(ch);
		}
	}

	void InteractiveConsole::handle_key_event(char ch)
	{
		const std::lock_guard<std::mutex> lock(_mtxOut);

		// handle special keys
		if (ch == '\n' || ch == '\r') // enter key
		{
			write_to_console("\n");

			std::string input = _curIn;
			_curIn.clear();

			// handle input (unlock before calling handler to avoid deadlock)
			_mtxOut.unlock();
			_handlingInput = true;
			_running = !_handleInput(*this, input); // _handleInput returns `true` for stop
			_handlingInput = false;
			_mtxOut.lock();

			if (_running) // only show prompt if still running
				display_prompt();
		}
		else if (ch == 127 || ch == 8) // backspace (DEL or BS)
		{
			if (!_curIn.empty())
			{
				_curIn.pop_back();
				// move cursor back, write space, move back again
				write_to_console("\b \b");
			}
		}
		else if (ch == 27) // escape sequence (arrow keys, etc.)
		{
			// read the rest of the escape sequence and ignore for now
			// this prevents arrow keys from printing garbage
			char seq[2];
			if (read(_stdin_fd, &seq[0], 1) > 0 && seq[0] == '[')
			{
				read(_stdin_fd, &seq[1], 1);
			}
		}
		else if (ch) // non-null character to print
		{
			_curIn += ch;
			write_to_console(std::string(1, ch));
		}
	}

	void InteractiveConsole::display_prompt()
	{
		write_to_console("> ");
	}

	void InteractiveConsole::clear_current_line()
	{
		// ANSI escape sequence: move to beginning of line and clear to end
		write_to_console("\r\033[K");
	}

	void InteractiveConsole::write_to_console(const std::string& text)
	{
		write(_stdout_fd, text.c_str(), text.size());

		// flush output to ensure it's displayed immediately
		fsync(_stdout_fd);
	}
}
#endif

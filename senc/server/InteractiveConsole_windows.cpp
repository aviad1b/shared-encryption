/*********************************************************************
 * \file   InteractiveConsole_windows.cpp
 * \brief  Implementation of InteractiveConsole class for windows.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#include "InteractiveConsole.hpp"

#ifdef SENC_WINDOWS

#include "../utils/Exception.hpp"

namespace senc::server
{
	InteractiveConsole::InteractiveConsole(std::function<bool(Self&, const std::string&)> handleInput)
		: _handleInput(handleInput),
		  _running(false), _handlingInput(false),
		  _hStdin(GetStdHandle(STD_INPUT_HANDLE)),
		  _hStdout(GetStdHandle(STD_OUTPUT_HANDLE))
	{
		if (INVALID_HANDLE_VALUE == _hStdin || INVALID_HANDLE_VALUE == _hStdout)
			throw utils::Exception("Failed to initialize console");
	}

	InteractiveConsole::~InteractiveConsole()
	{
		stop_inputs();
	}

	void InteractiveConsole::start_inputs()
	{
		if (_running.exchange(true))
			return; // already running

		DWORD oldMode = 0;

		// save original console mode and set to raw input mode
		GetConsoleMode(_hStdin, &oldMode);
		SetConsoleMode(_hStdin, 0);

		// Display initial prompt
		display_prompt();

		// run input loop on this thread
		input_loop();

		// restore console mode when done
		SetConsoleMode(_hStdin, oldMode);
	}

	void InteractiveConsole::print(const std::string& msg)
	{
		// if input loop is not running or mid-handling input, do regular print
		// otherwise, do complex logic
		if (!_running || _handlingInput)
			write_to_console(msg + "\r\n");
		else
		{
			const std::lock_guard<std::mutex> lock(_mtxOut);

			// clear and print
			clear_current_line();
			write_to_console(msg + "\r\n");

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
		INPUT_RECORD inputRecord{};
		DWORD numRead = 0;

		while (_running)
		{
			// use a timeout so we can check _running periodically
			DWORD waitResult = WaitForSingleObject(_hStdin, 100);
			if (WAIT_OBJECT_0 != waitResult)
				continue;

			if (!ReadConsoleInput(_hStdin, &inputRecord, 1, &numRead))
				break;

			if (0 == numRead)
				continue;

			// only process key events
			if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown)
				handle_key_event(inputRecord.Event.KeyEvent);
		}
	}

	void InteractiveConsole::handle_key_event(const KEY_EVENT_RECORD& keyEvent)
	{
		const std::lock_guard<std::mutex> lock(_mtxOut);

		const char ch = keyEvent.uChar.AsciiChar;
		const WORD vkCode = keyEvent.wVirtualKeyCode;

		if (ch == 0 && vkCode != VK_RETURN && vkCode != VK_BACK)
			return;

		switch (vkCode)
		{
		case VK_RETURN: // enter key
		{
			write_to_console("\r\n");

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
			break;
		}
		case VK_BACK: // backspace key
		{
			if (!_curIn.empty())
			{
				_curIn.pop_back();

				// move cursor back, write space, move back again
				write_to_console("\b \b");
			}
			break;
		}
		default:
		{
			_curIn += ch;
			write_to_console(std::string(1, ch));
			break;
		}
		}
	}

	void InteractiveConsole::display_prompt()
	{
		write_to_console("> ");
	}

	void InteractiveConsole::clear_current_line()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		if (!GetConsoleScreenBufferInfo(_hStdout, &csbi))
			return;

		// move cursor to beginning of current line
		COORD cursorPos = csbi.dwCursorPosition;
		cursorPos.X = 0;
		SetConsoleCursorPosition(_hStdout, cursorPos);

		// calculate number of characters to clear (from cursor to end of line)
		DWORD lineLength = csbi.dwSize.X;
		DWORD charsWritten = 0;

		// fill with spaces
		FillConsoleOutputCharacter(_hStdout, ' ', lineLength, cursorPos, &charsWritten);

		// reset attributes for the cleared area
		FillConsoleOutputAttribute(_hStdout, csbi.wAttributes, lineLength, cursorPos, &charsWritten);

		// move cursor back to beginning of line
		SetConsoleCursorPosition(_hStdout, cursorPos);
	}

	void InteractiveConsole::write_to_console(const std::string& text)
	{
		DWORD written = 0;
		WriteConsoleA(_hStdout, text.c_str(), static_cast<DWORD>(text.size()), &written, nullptr);

		// flushoutput to ensure it's displayed immediately
		FlushFileBuffers(_hStdout);
	}
}
#endif

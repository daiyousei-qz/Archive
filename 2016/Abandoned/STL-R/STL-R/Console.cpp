#include "Console.h"
#include "windows.h"
#include <string>

using namespace eds;
using std::string;

HANDLE _hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE _hStdIn = GetStdHandle(STD_INPUT_HANDLE);

#pragma region Beep

void Console::Beep()
{
	::Beep(800, 200);
}

void Console::Beep(int freq, int duration)
{
	::Beep(freq, duration);
}

#pragma endregion

#pragma region Title

string &&Console::GetTitle()
{
	constexpr size_t TitleBufSize = 128;
	char buf[TitleBufSize];
	size_t sz = GetConsoleTitleA(buf, TitleBufSize);

	return std::move(string(buf, sz));
}

void Console::SetTitle(const string &s)
{
	SetConsoleTitleA(s.c_str());
}

#pragma endregion

#pragma region Display

void Console::SetDisplayMode(ConsoleDisplayMode mode)
{
	if (mode == ConsoleDisplayMode::Fullscreen)
	{
		SetConsoleDisplayMode(_hStdOut, CONSOLE_FULLSCREEN, nullptr);
	}
	else if (mode == ConsoleDisplayMode::Windowed)
	{
		SetConsoleDisplayMode(_hStdOut, CONSOLE_WINDOWED_MODE, nullptr);
	}
}

void Console::SetForegroundColor(ConsoleColor color)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(_hStdOut, &csbi);

	// assert color is valid
	WORD attr = (csbi.wAttributes & 0xfff0) | static_cast<WORD>(color);
	SetConsoleTextAttribute(_hStdOut, attr);
}

void Console::SetBackgroundColor(ConsoleColor color)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(_hStdOut, &csbi);

	// assert color is valid
	WORD attr = (csbi.wAttributes & 0xff0f) | (static_cast<WORD>(color) << 4);
	SetConsoleTextAttribute(_hStdOut, attr);
}

void Console::SetScreenMargin(short left, short top, short right, short bottom)
{
	SMALL_RECT rect{ left,top,right,bottom };
	SetConsoleWindowInfo(_hStdOut, true, &rect);
}

#pragma endregion

#pragma region ScreenBufferInfo

ConsoleCoord Console::GetWindowSize()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(_hStdOut, &csbi);

	return ConsoleCoord{ csbi.dwMaximumWindowSize.X, csbi.dwMaximumWindowSize.Y };
}

ConsoleCoord Console::GetBufferSize()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(_hStdOut, &csbi);

	return ConsoleCoord{ csbi.dwSize.X ,csbi.dwSize.Y };
}

ConsoleCoord Console::GetCursorPosition()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(_hStdOut, &csbi);

	return ConsoleCoord{ csbi.dwCursorPosition.X ,csbi.dwCursorPosition.Y };
}

void Console::SetBufferSize(short x, short y)
{
	SetConsoleScreenBufferSize(_hStdOut, COORD{ x, y });
}

void Console::SetCursorPosition(short x, short y)
{
	SetConsoleCursorPosition(_hStdOut, COORD{ x, y });
}

void Console::SetCursorVisibility(bool visible)
{
	// retrive cursor information
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(_hStdOut, &cci);

	// set visibility
	cci.bVisible = visible;
	SetConsoleCursorInfo(_hStdOut, &cci);
}

void Console::Clear()
{
	COORD coordScreen = { 0, 0 };    // home for the cursor 
	DWORD cCharsWritten;
	DWORD dwConSize;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	// Get the number of character cells in the current buffer. 
	if (!GetConsoleScreenBufferInfo(_hStdOut, &csbi)) return;

	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	// Fill the entire screen with blanks.
	if (!FillConsoleOutputCharacterA(
		_hStdOut,         // Handle to console screen buffer 
		(TCHAR) ' ',     // Character to write to the buffer
		dwConSize,       // Number of cells to write 
		coordScreen,     // Coordinates of first cell 
		&cCharsWritten)) // Receive number of characters written
	{
		return;
	}

	// Get the current text attribute.
	if (!GetConsoleScreenBufferInfo(_hStdOut, &csbi)) return;


	// Set the buffer's attributes accordingly.
	if (!FillConsoleOutputAttribute(
		_hStdOut,         // Handle to console screen buffer 
		csbi.wAttributes, // Character attributes to use
		dwConSize,        // Number of cells to set attribute 
		coordScreen,      // Coordinates of first cell 
		&cCharsWritten))  // Receive number of characters written
	{
		return;
	}

	// Put the cursor at its home coordinates.
	SetCursorPosition(0, 0);
}

#pragma endregion

#pragma region Write

void Console::Write(const std::string &s)
{
	DWORD written;
	WriteConsoleA(_hStdOut, s.c_str(), s.size(), &written, nullptr);
}

void Console::WriteLine(const std::string &s)
{
	Write(s);
	Write("\r\n");
}

#pragma endregion

#pragma region Read

size_t Console::GetInputRecordNumber()
{
	DWORD num;
	GetNumberOfConsoleInputEvents(_hStdIn, &num);

	return num;
}

ConsoleKey Console::ReadKey()
{
	INPUT_RECORD ir;
	DWORD num = 0;
	ReadConsoleInputA(_hStdIn, &ir, 1, &num);

	if (ir.EventType != KEY_EVENT || ir.Event.KeyEvent.bKeyDown || num == 0)
		return ConsoleKey::InvalidKey;
	else
		return static_cast<ConsoleKey>(ir.Event.KeyEvent.wVirtualKeyCode);
}

#pragma endregion

void Console::Render(const ConsoleScreenBuffer &buffer)
{
	SMALL_RECT rect = SMALL_RECT{ 0,0, buffer._x, buffer._y };

	WriteConsoleOutputA(_hStdOut,
		reinterpret_cast<const CHAR_INFO*>(buffer._buf),
		COORD{ buffer._x, buffer._y },
		COORD{ 0, 0 },
		&rect);
}

void Console::Sleep(int duration)
{
	::Sleep(duration);
}

void Console::WaitForInput(int interval)
{
	WaitForSingleObject(_hStdIn, interval);
}

#pragma region ScreenBuffer

// this struct defined in WIN API
struct ConsoleScreenBuffer::CharInfo
{
	union
	{
		char AnsiCh;
		wchar_t UnicodeCh;
	};
	short CellAttribute;
};

ConsoleScreenBuffer::ConsoleScreenBuffer(short x, short y) : _x(x), _y(y)
{
	// assume x>0, y>0
	_buf = new CharInfo[x*y];
	_foreground = ConsoleColor::Gray;
	_background = ConsoleColor::Black;

	Clear();
}

ConsoleScreenBuffer::~ConsoleScreenBuffer()
{
	delete[] _buf;
}

void ConsoleScreenBuffer::SetDefaultColor(ConsoleColor fore, ConsoleColor back)
{
	_foreground = fore;
	_background = back;
}

void ConsoleScreenBuffer::Clear()
{
	for (int i = 0; i < _x * _y; ++i)
	{
		_buf[i].AnsiCh = ' ';
		_buf[i].CellAttribute = (static_cast<short>(_background) << 4) | static_cast<short>(_foreground);
	}
}

void ConsoleScreenBuffer::Print(char ch, ConsoleCoord position)
{
	auto &cell = _buf[_x * position.Y + position.X];

	cell.AnsiCh = ch;
	cell.CellAttribute = (cell.CellAttribute & 0xfff0) | static_cast<short>(_foreground);
}

void ConsoleScreenBuffer::Print(const string &s, ConsoleCoord position)
{
	for (char ch : s)
	{
		// print char
		Print(ch, position);

		// advance position
		++position.X;
		if (position.X == _x)
		{
			++position.Y;
			position.X = 0;
		}
	}
}

void ConsoleScreenBuffer::Draw(ConsoleCoord position, ConsoleColor backColor)
{
	auto &cell = _buf[_x * position.Y + position.X];

	cell.AnsiCh = ' ';
	cell.CellAttribute = (cell.CellAttribute & 0xff0f) | (static_cast<short>(backColor) << 4);
}

#pragma endregion
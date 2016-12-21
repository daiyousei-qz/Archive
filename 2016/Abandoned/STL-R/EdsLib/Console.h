#pragma once
#ifndef STLR_CONSOLE_H
#define STLR_CONSOLE_H

#include "Basic.h"
#include <string>

namespace eds
{
	enum class ConsoleKey;
	enum class ConsoleColor;
	enum class ConsoleDisplayMode;

	struct ConsoleCoord;
	class ConsoleScreenBuffer;

	class Console : Unconstructible
	{
	public:
		Console() = delete;

		// beep
		static void Beep();
		static void Beep(int freq, int duration);

		// title
		static std::string &&GetTitle();
		static void SetTitle(const std::string &name);

		// display
		static void SetDisplayMode(ConsoleDisplayMode mode);
		static void SetForegroundColor(ConsoleColor color);
		static void SetBackgroundColor(ConsoleColor color);
		static void SetScreenMargin(short left, short top, short right, short bottom);

		// screen buffer
		static ConsoleCoord GetWindowSize();
		static ConsoleCoord GetBufferSize();
		static ConsoleCoord GetCursorPosition();

		static void SetBufferSize(short x, short y);
		static void SetCursorPosition(short x, short y);

		static void SetCursorVisibility(bool visible);
		static void Clear();

		// character io
		static void Write(const std::string &s);
		static void WriteLine(const std::string &s);

		//static int Read();
		//static ConsoleKey ReadKey();
		//static void ReadLine();

		// low-level control
		static void Render(const ConsoleScreenBuffer &buffer);
		static ConsoleKey ReadKey();
		static size_t GetInputRecordNumber();

		static void Sleep(int duration); // in milliseconds
		static void WaitForInput(int interval); // in milliseconds
	};

	// the value of this enumeration coresponds to WINAPI
	enum class ConsoleKey
	{
		InvalidKey = 0,

		LB = 0x01, // Left mouse button
		RB = 0x02, // Right mouse button
		Cancel = 0x03,
		MB = 0x04, // Middle mouse button
		XB1 = 0x05, // X1 mouse button
		XB2 = 0x06,

		Backspace = 0x08,
		Tab = 0x09,

		Clear = 0x0c,
		Enter = 0x0d,

		Shift = 0x10,
		Ctrl = 0x11,
		Alt = 0x12,
		Pause = 0x13,
		CapLocks = 0x14,
		IMEKana = 0x15,
		IMEHanguel = 0x15,

		IMEJunja = 0x17,
		IMEFinal = 0x18,
		IMEHanja = 0x19,
		IMEKanji = 0x19,

		Esc = 0x1b,
		IMEConvert = 0x1c,
		IMENonconvert = 0x1d,
		IMEAccept = 0x1e,
		IMEModeChangeReq = 0x1f,
		SpaceBar = 0x20,

		PageUp = 0x21,
		PageDown = 0x22,
		End = 0x23,
		Home = 0x24,
		LeftArrow = 0x25,
		UpArrow = 0x26,
		RightArrow = 0x27,
		DownArrow = 0x28,
		Select = 0x29,
		Print = 0x2A,
		Execute = 0x2b,
		PrintScreen = 0x2c,
		Insert = 0x2d,
		Delete = 0x2e,
		Help = 0x2f,

		D0 = 0x30, // degit 0
		D1 = 0x31,
		D2 = 0x32,
		D3 = 0x33,
		D4 = 0x34,
		D5 = 0x35,
		D6 = 0x36,
		D7 = 0x37,
		D8 = 0x38,
		D9 = 0x39,

		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4a,
		K = 0x4b,
		L = 0x4c,
		M = 0x4d,
		N = 0x4e,
		O = 0x4f,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5a,

		LWin = 0x5b,
		RWin = 0x5c,
		Apps = 0x5d,

		Sleep = 0x5f,
		Numpad0 = 0x60,
		Numpad1 = 0x61,
		Numpad2 = 0x62,
		Numpad3 = 0x63,
		Numpad4 = 0x64,
		Numpad5 = 0x65,
		Numpad6 = 0x66,
		Numpad7 = 0x67,
		Numpad8 = 0x68,
		Numpad9 = 0x69,

		Multiply = 0x6a,
		Add = 0x6b,
		Saperator = 0x6c,
		Subtract = 0x6d,
		Decimal = 0x6e,
		Divide = 0x6f,
		F1 = 0x70,
		F2 = 0x71,
		F3 = 0x72,
		F4 = 0x73,
		F5 = 0x74,
		F6 = 0x75,
		F7 = 0x76,
		F8 = 0x77,
		F9 = 0x78,
		F10 = 0x79,
		F11 = 0x7a,
		F12 = 0x7b,
		F13 = 0x7c,
		F14 = 0x7d,
		F15 = 0x7e,
		F16 = 0x7f,
		F17 = 0x80,
		F18 = 0x81,
		F19 = 0x82,
		F20 = 0x83,
		F21 = 0x84,
		F22 = 0x85,
		F23 = 0x86,
		F24 = 0x87,

		NumLock = 0x90,
		Scroll = 0x91,

		LShift = 0xa0,
		RShift = 0xa1,
		LCtrl = 0xa2,
		RCtrl = 0xa3,
		LMenu = 0xa4,
		RMenu = 0xa5,

		//...
	};

	// the value of this enumeration coresponds to WINAPI
	enum class ConsoleColor
	{
		Black = 0b0000,
		Blue = 0b1001,
		Green = 0b1010,
		Red = 0b1100,
		Cyan = 0b1011,
		Rose = 0b1101,
		Yellow = 0b1110,
		Gray = 0b1111,
		DeepBlue = 0b0001,
		DeepGreen = 0b0010,
		DeepRed = 0b0100,
		DeepCyan = 0b0011,
		DeepRose = 0b0101,
		DeepYellow = 0b0110,
		DeepGray = 0b0111
	};

	enum class ConsoleDisplayMode
	{
		Fullscreen,
		Windowed
	};

	struct ConsoleCoord
	{
		short X;
		short Y;
	};

	class ConsoleScreenBuffer
	{
		// this struct defined in WIN API
		struct CharInfo;

		CharInfo *_buf;
		short _x, _y;
		ConsoleColor _foreground;
		ConsoleColor _background;

		friend class Console;

	public:
		ConsoleScreenBuffer(short x, short y);
		ConsoleScreenBuffer(ConsoleScreenBuffer &) = delete;
		~ConsoleScreenBuffer();

		ConsoleCoord GetScreenSize() const { return ConsoleCoord{ _x, _y }; }

		void SetDefaultColor(ConsoleColor fore, ConsoleColor back);
		void Clear();

		void Print(char ch, ConsoleCoord position);
		void Print(const std::string &s, ConsoleCoord position);
		void Draw(ConsoleCoord position, ConsoleColor backColor);
	};
}

#endif // !STLR_CONSOLE_H
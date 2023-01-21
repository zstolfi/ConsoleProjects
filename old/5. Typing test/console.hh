#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <algorithm>
#include <chrono>

/*
TODO:
	[ ]		save the previous console default, and return it
*/


using std::chrono::system_clock;
typedef system_clock::time_point timePoint;

class ConsoleDisplay {
public:
	short WIDTH;
	short HEIGHT;
	bool running;

	CHAR_INFO* screenBuffer;

	bool inFocus = true;

	int frameCount;
	float Δtime;
	float totalTime;

	bool keys[256] = { 0 };
	char keysDiff[256] = { 0 }; //  '1' means pressed this frame
	                            // '-1' means released
protected:
	std::wstring title;
	COORD bufferCoord;
	SMALL_RECT windowRect;

	HANDLE consoleWrite;
	HANDLE consoleRead;
	HWND consoleWindow = GetConsoleWindow();

	timePoint startTime = system_clock::now();
	timePoint time0 = startTime;
	timePoint time1 = startTime;

	bool keysPrev[256] = { 0 };

public:
	virtual void setup() {}

	virtual void run() {}

	void initConsole(short w = 80, short h = 40, short fontW = 8, short fontH = 16) {
		WIDTH = w, HEIGHT = h;
		running = true;

		consoleWrite = GetStdHandle(STD_OUTPUT_HANDLE);
		consoleRead  = GetStdHandle(STD_INPUT_HANDLE);

		getOriginalValues();

		title = L"Console App";
		SetConsoleTitleW(title.c_str());

		// disable cursor
		CONSOLE_CURSOR_INFO cursor = {1, false};
		SetConsoleCursorInfo(consoleWrite, &cursor);
		SetConsoleMode(consoleRead, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT);

		// window size
		windowRect = {0, 0, WIDTH-1, HEIGHT-1};
		SetConsoleWindowInfo(consoleWrite, true, &windowRect);
		bufferCoord = {WIDTH, HEIGHT};
		SetConsoleScreenBufferSize(consoleWrite, bufferCoord);
		SetConsoleActiveScreenBuffer(consoleWrite);

		// font size
		setFont(fontW, fontH);

		// init variables
		screenBuffer = new CHAR_INFO[WIDTH * HEIGHT];
		memset(screenBuffer, 0, sizeof(CHAR_INFO) * WIDTH * HEIGHT);
		memset(keys, 0, sizeof(bool) * 256);

		// stylize window
		consoleWindow = GetConsoleWindow();
		LONG toRemove =  WS_MAXIMIZEBOX | WS_SIZEBOX;
		LONG toAdd = 0;
		SetWindowLong(consoleWindow, GWL_STYLE, origWindowStyle & ~toRemove | toAdd);
	}

	void setFont(wchar_t* fontName, short fontW, short fontH) {
		CONSOLE_FONT_INFOEX font;
		font.cbSize = sizeof(font);
		font.nFont = 0;
		font.dwFontSize.X = fontW;
		font.dwFontSize.Y = fontH;
		font.FontFamily = FF_DONTCARE;
		font.FontWeight = FW_NORMAL;
		wcscpy_s(font.FaceName, fontName);
		SetCurrentConsoleFontEx(consoleWrite, false, &font);
	}
	void setFont(short w, short h) { setFont(L"Consolas", w, h); }

	void drawConsole() {
		wchar_t titleBuffer[256];
		swprintf_s(titleBuffer, 256, L"%s - FPS: %4.1f", title.c_str(), 1.0/Δtime);

		WriteConsoleOutputW(consoleWrite, screenBuffer, {WIDTH,HEIGHT}, {0,0}, &windowRect);
		SetConsoleTitleW(titleBuffer);
	}

	void updateTime() {
		time1 = system_clock::now();
		totalTime = calcDuration(time1 , startTime);
		Δtime     = calcDuration(time1 , time0);

		time0 = time1;
		frameCount++;
	}


	void updateKeys() {
		DWORD events = 0;
		GetNumberOfConsoleInputEvents(consoleRead, &events);
		if (events > 0) {
			INPUT_RECORD input[events];
			ReadConsoleInput(consoleRead, input, events, &events);

			for (DWORD i = 0; i < events; i++) {
				switch (input[i].EventType) {
					case KEY_EVENT: {
						WORD keyCode = input[i].Event.KeyEvent.wVirtualKeyCode;
						bool state   = input[i].Event.KeyEvent.bKeyDown;

						keys[keyCode] = state;
						// if (keyCode == VK_ESCAPE) { exit(); }
						
					} break;

					case MOUSE_EVENT: {
						// TODO: add mouse input
					} break;

					case FOCUS_EVENT: {
						inFocus = input[i].Event.FocusEvent.bSetFocus;
					} break;

					default:
						break;
				}
			}
		}

		for (int i=0; i < 256; i++) {
			keysDiff[i] = keys[i] - keysPrev[i];

			keysPrev[i] = keys[i];
		}
	}

	void exit() {
		running = false;
	}


	void start() {
		// It's up to User to initConsole()
		setup();

		while (running) {
			run();
			drawConsole();
			updateKeys();
			updateTime();
		}

		returnOrigConsole();
	}

private:


	// keep track of all the original styles!
	CONSOLE_CURSOR_INFO origCursor;
	DWORD origMode;

	CONSOLE_SCREEN_BUFFER_INFOEX origBufferInfo;

	CONSOLE_FONT_INFOEX origFont;

	LONG origWindowStyle;

	void getOriginalValues() { // TODO: get this shit to work
		GetConsoleCursorInfo(consoleWrite, &origCursor);
		GetConsoleMode(consoleRead, &origMode);
		GetConsoleScreenBufferInfoEx(consoleWrite, &origBufferInfo);
		GetCurrentConsoleFontEx(consoleWrite, false, &origFont);

		origWindowStyle = GetWindowLong(consoleWindow, GWL_STYLE);
	}

	void returnOrigConsole() {
		// SetConsoleCursorInfo(consoleWrite, &origCursor);
		// SetConsoleMode(consoleRead, origMode);
		// SetConsoleScreenBufferInfoEx(consoleWrite, &origBufferInfo);
		// SetCurrentConsoleFontEx(consoleWrite, false, &origFont);
		// SetWindowLong(consoleWindow, GWL_STYLE, origWindowStyle);

		// SetConsoleActiveScreenBuffer(consoleWrite);
	}


	float calcDuration(timePoint t1, timePoint t0) {
		return std::chrono::duration<float>(t1 - t0).count();
	}
};
#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <algorithm>
#include <chrono>

auto time0 = std::chrono::system_clock::now();
auto time1 = std::chrono::system_clock::now();

class ConsoleDisplay {
public:
	short WIDTH;
	short HEIGHT;
	bool running;

	CHAR_INFO* screenBuffer;

	int frameCount;
	float Δtime;

protected:
	std::wstring title;
	COORD bufferCoord;
	SMALL_RECT windowRect;

	HANDLE consoleWrite;
	HANDLE consoleRead;

public:
	virtual void setup() {}

	virtual void run() {}

	void initConsole(short w = 80, short h = 40, short fontW = 8, short fontH = 16) {
		WIDTH = w, HEIGHT = h;
		running = true;

		consoleWrite = GetStdHandle(STD_OUTPUT_HANDLE);
		consoleRead  = GetStdHandle(STD_INPUT_HANDLE);

		title = L"Console App";
		SetConsoleTitleW(title.c_str());

		// disable cursor
		CONSOLE_CURSOR_INFO cursor = {1, false};
		SetConsoleCursorInfo(consoleWrite, &cursor);

		windowRect = {0, 0, WIDTH-1, HEIGHT-1};
		SetConsoleWindowInfo(consoleWrite, true, &windowRect);
		bufferCoord = {WIDTH, HEIGHT};
		SetConsoleScreenBufferSize(consoleWrite, bufferCoord);
		SetConsoleActiveScreenBuffer(consoleWrite);

		screenBuffer = new CHAR_INFO[WIDTH * HEIGHT];
		memset(screenBuffer, 0, sizeof(CHAR_INFO) * WIDTH * HEIGHT);

		memset(keys, 0, sizeof(bool) * 256);

		// stylize window
		HWND consoleWindow = GetConsoleWindow();
		LONG toRemove =  WS_MAXIMIZEBOX | WS_SIZEBOX;
		LONG toAdd = 0;
		SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~toRemove | toAdd);
	}

	void updateConsole() {
		wchar_t titleBuffer[256];
		swprintf_s(titleBuffer, 256, L"%s - FPS: %4.1f", title.c_str(), 1.0/Δtime);

		WriteConsoleOutputW(consoleWrite, screenBuffer, {WIDTH,HEIGHT}, {0,0}, &windowRect);
		SetConsoleTitleW(titleBuffer);
	}

	void updateTime() {
		time1 = std::chrono::system_clock::now();
		Δtime = std::chrono::duration<float>(time1 - time0).count();
		time0 = time1;

		frameCount++;
	}

	bool keys[256];

	void updateKeys() {
		DWORD events = 0;
		GetNumberOfConsoleInputEvents(consoleRead, &events);
		if (events > 0) {
			INPUT_RECORD* inputs = new INPUT_RECORD[events];
			ReadConsoleInput(consoleRead, inputs, events, &events);

			for (DWORD i = 0; i < events; i++) {
				switch (inputs[i].EventType) {
					case KEY_EVENT: {
						WORD keyCode = inputs[i].Event.KeyEvent.wVirtualKeyCode;
						bool state   = inputs[i].Event.KeyEvent.bKeyDown;

						keys[keyCode] = state;
						if (keyCode == VK_ESCAPE) { exit(); }
						
					} break;

					case MOUSE_EVENT: {
						// TODO: add mouse input
					} break;

					default:
						break;
				}
			}
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
			updateConsole();
			updateKeys();
			updateTime();
		}
	}

};
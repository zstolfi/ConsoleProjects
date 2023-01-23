#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <vector>
#include <chrono>



using std::chrono::steady_clock;
using seconds = std::chrono::duration<double, std::ratio<1>>;

// TODO: system to restore original console style
class ConsoleWindow {
public: /* Public Constructor */
	struct initSettings { short width, height, fontW, fontH; };
	ConsoleWindow(std::wstring title, initSettings s)
	: title{title} 
	, width{s.width}, height{s.height}
	, windowRect{0,0, width-1, height-1} {
		screenBuffer.resize(s.width * s.height);

		consoleIn  = GetStdHandle(STD_INPUT_HANDLE );
		consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		window     = GetConsoleWindow();

		// set title
		SetConsoleTitleW(title.c_str());

		// disable cursor
		CONSOLE_CURSOR_INFO newCursor = {1, false};
		SetConsoleCursorInfo(consoleOut, &newCursor);
		SetConsoleMode(consoleIn, ENABLE_EXTENDED_FLAGS // < turns off cursor
		                        | ENABLE_WINDOW_INPUT);
		// window size
		SetConsoleWindowInfo(consoleOut, true, &windowRect);
		SetConsoleScreenBufferSize(consoleOut, COORD{width, height});
		SetConsoleActiveScreenBuffer(consoleOut);

		// font size
		CONSOLE_FONT_INFOEX newFont = { .cbSize = sizeof(newFont) };
		GetCurrentConsoleFontEx(consoleOut, false, &newFont);
		newFont.dwFontSize = COORD{s.fontW, s.fontH};
		SetCurrentConsoleFontEx(consoleOut, false, &newFont);

		// stylize window
		LONG_PTR styleAdd = 0;
		LONG_PTR styleRemove = WS_MAXIMIZEBOX | WS_SIZEBOX;
		LONG_PTR oldStyle = GetWindowLongPtr(window, GWL_STYLE);
		SetWindowLongPtr(window, GWL_STYLE, oldStyle & ~styleRemove | styleAdd);
		ShowScrollBar(window, SB_BOTH, false);
	}

public: /* Methods For Main Function */
	bool quit() { return !running; }

	void start() { setup(); }

	void run() {
		if (!noDraw || frameCount == 0) draw();
		update();
		updateConsole();
		readEvents();
		updateKeys();
		updateTime();
	}


protected: /* App-only Methods */
	virtual void setup() {}  // initialize game veriables
	virtual void update() {} // handle input / update state
	virtual void draw() {}   // draw to the screen

	void setTitle(std::wstring newTitle) { title = newTitle; }	
	void quitConsole() { running = false; }

	bool noDraw = false; // true = draw on frame 0

	// using timePoint = steady_clock::time_point;
	unsigned frameCount = 0;
	seconds totalTime{0};
	seconds Δtime    {1e-3};

	auto& getScreenBuffer() { return screenBuffer; }
	unsigned getWidth () { return width; }
	unsigned getHeight() { return height; }

	enum class keyState { NONE, PRESSED, HELD, RELEASED }; // https://files.catbox.moe/6og5li.png
	std::array<keyState,256> keys = {};

	// void writeChar(wchar_t c, WORD col) {}

	// void replaceScreen(std::vector<CHAR_INFO>&& newBuffer) {}


private: /* Internal Workings */
	std::wstring title = L"Console Application";
	short width, height;
	bool running = true;
	bool inFocus = true;

	std::vector<CHAR_INFO> screenBuffer = {};
	SMALL_RECT windowRect;

	HANDLE consoleIn;
	HANDLE consoleOut;
	HWND   window;

	steady_clock::time_point startTime = steady_clock::now();
	steady_clock::time_point time0     = startTime;
	steady_clock::time_point time1     = startTime;

	std::array<bool,256> keysRaw  = {};
	std::array<bool,256> keysPrev = {};

	void updateConsole() {
		wchar_t titleBuffer[256];
		double fps = std::chrono::seconds{1} / Δtime;
		swprintf_s(titleBuffer, 256, L"%s - FPS: %4.1f", title.c_str(), fps);

		WriteConsoleOutputW(consoleOut, screenBuffer.data(), {width,height}, {0,0}, &windowRect);
		SetConsoleTitleW(titleBuffer);
	}

	void readEvents() {
		DWORD numEvents = 0;
		GetNumberOfConsoleInputEvents(consoleIn, &numEvents);
		if (numEvents == 0) { return; }
		INPUT_RECORD input[numEvents];
		ReadConsoleInput(consoleIn, input, numEvents, &numEvents);

		for (std::size_t i = 0; i < numEvents; i++) {
			switch (input[i].EventType) {
			case KEY_EVENT: {
				WORD keyCode = input[i].Event.KeyEvent.wVirtualKeyCode;
				bool pressed = input[i].Event.KeyEvent.bKeyDown;
				keysRaw[keyCode] = pressed;
				} break;

				case MOUSE_EVENT: break;

			case FOCUS_EVENT: {
				inFocus = input[i].Event.FocusEvent.bSetFocus;
				if (!inFocus) { keysRaw.fill(false); }
				} break;
		} }
	}

	void updateKeys() {
		for (std::size_t i=0; i < 256; i++) {
			int diff = keysRaw[i] - keysPrev[i];
			/**/ if (diff == +1) { keys[i] = keyState::PRESSED; }
			else if (diff == -1) { keys[i] = keyState::RELEASED; }
			else if (keysRaw[i]) { keys[i] = keyState::HELD; }
			else                 { keys[i] = keyState::NONE; }

			keysPrev[i] = keysRaw[i];
	} }

	void updateTime() {
		time1 = steady_clock::now();

		#define duration std::chrono::duration_cast<seconds>
		totalTime = duration(time1 - startTime);\
		Δtime     = duration(time1 - time0);
		#undef duration

		time0 = time1;
		frameCount++;
	}

public:
	void error(std::wstring message=L"Unkown error") {
		running = false;

		wchar_t str[256];
		swprintf_s(str, 256, L"Error!: \n%s", message.c_str());
		for (std::size_t i=0, x=0, y=0; i < 256 && str[i] != L'\0'; i++) {
			switch (str[i]) {
			case L'\n': { x = 0; y++; } break;

			default: {
				if ((0 <= x&&x < width) && (0 <= y&&y < height)) {
					screenBuffer[y * width + x].Char.UnicodeChar = str[i];
					screenBuffer[y * width + x].Attributes = 0x000C;
				}
				x++; } break;
		} }

		updateConsole();
		while (true) {}
	}
	void error(std::string msg) { error(std::wstring(msg.begin(), msg.end())); }
};
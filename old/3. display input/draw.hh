#pragma once

#include "console.hh"

namespace Draw {

	namespace { // anonymous
		ConsoleDisplay* ConsoleTarget;
		CHAR_INFO* screenBuffer;
		short width;
		short height;

		bool bounds(int x, int y) {
			return 0 <= x && x < width
			    && 0 <= y && y < height;
		}
	}

	void setConsole(ConsoleDisplay* console) {
		ConsoleTarget = console;
		screenBuffer  = console->screenBuffer;
		width   = console->WIDTH;
		height  = console->HEIGHT;
	}



	void clearScreen() {
		memset(screenBuffer, 0, sizeof(CHAR_INFO) * width * height);
	}

	void rect(int startX, int startY, int endX, int endY, wchar_t character = L'█', WORD color = 0x000F) {
		for (int y = startY; y < endY; y++) {
		for (int x = startX; x < endX; x++) {
			if (!bounds(x,y)) { continue; }
			screenBuffer[y * width + x].Char.UnicodeChar = character;
			screenBuffer[y * width + x].Attributes = color;
		} }
	}

}
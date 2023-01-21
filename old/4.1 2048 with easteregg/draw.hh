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

		const WORD defColor = 0x000F;
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

	void rect(int startX, int startY, int endX, int endY, wchar_t character = L'█', WORD color = defColor) {
		for (int y = startY; y < endY; y++) {
		for (int x = startX; x < endX; x++) {
			if (!bounds(x,y)) { continue; }
			screenBuffer[y * width + x].Char.UnicodeChar = character;
			screenBuffer[y * width + x].Attributes = color;
		} }
	}

	void text(std::wstring string, WORD color, int startX, int startY) {
		int x = startX;
		int y = startY;
		for (int i=0; i < string.size(); i++) {
			wchar_t c = string[i];
			if (c != '\n') {
				if (bounds(x,y)) {
					screenBuffer[y * width + x].Char.UnicodeChar = c;
					screenBuffer[y * width + x].Attributes = color;
				}
				x++;
			} else {
				x = startX;
				y++;
			}
		}
	}

		// only works for single lines (right now)
	void text_CenterX(std::wstring string, WORD color, int centerX, int startY) {
		int length = string.size();
		text(string, color, centerX-length/2, startY);
	}

		// also only works for single lines
	void text_RightX(std::wstring string, WORD color, int endX, int startY) {
		int length = string.size();
		text(string, color, endX-length, startY);
	}



	// alt versions
	void text(std::wstring str, int x, int y) { text(str, defColor, x, y); }
	void text(std::wstring str)                { text(str, defColor, 0, 0); }

	void text_CenterX(std::wstring str, int x, int y)  { text_CenterX(str, defColor, x, y); }
	// void text_CenterX(std::wstring str, WORD c, int y) { text_CenterX(str, c, width/2, y); }
	void text_CenterX(std::wstring str, int y)         { text_CenterX(str, defColor, width/2, y); }
	void text_CenterX(std::wstring str)                { text_CenterX(str, defColor, width/2, 0); }

	void text_RightX(std::wstring str, int x, int y)  { text_RightX(str, defColor, x, y); }
	// void text_RightX(std::wstring str, WORD c, int y) { text_RightX(str, c, width/2, y); }
	void text_RightX(std::wstring str, int y)         { text_RightX(str, defColor, width-1, y); }
	void text_RightX(std::wstring str)                { text_RightX(str, defColor, width-1, 0); }


}
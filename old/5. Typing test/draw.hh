#pragma once

#include "console.hh"
#include <string>

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

		std::wstring toWstring(std::string str) {
			return std::wstring(str.begin(), str.end());
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

	void fill(wchar_t character = L' ', WORD color = defColor) {
		for (int y=0; y < height; y++) {
		for (int x=0; x < width ; x++) {
			screenBuffer[y * width + x].Char.UnicodeChar = character;
			screenBuffer[y * width + x].Attributes = color;
		} }
	}

	void rect(int startX, int startY, int endX, int endY, wchar_t character = L'█', WORD color = defColor) {
		for (int y = startY; y < endY; y++) {
		for (int x = startX; x < endX; x++) {
			if (!bounds(x,y)) { continue; }
			screenBuffer[y * width + x].Char.UnicodeChar = character;
			screenBuffer[y * width + x].Attributes = color;
		} }
	}

	void character(wchar_t c, int x, int y, WORD color) {
		if (!bounds(x,y)) { return; }
		screenBuffer[y * width + x].Char.UnicodeChar = c;
		screenBuffer[y * width + x].Attributes = color;
	}

	// void character(char c, int x, int y, WORD color) {
	// 	if (!bounds(x,y)) { return; }
	// 	screenBuffer[y * width + x].Char.UnicodeChar = (wchar_t)c;
	// 	screenBuffer[y * width + x].Attributes = color;
	// }

	template<bool withNewLines = true, int offset = 0>
	void iterateText(auto string, auto&& func) {
		int x = 0, y = 0;
		for (int i=0; i < string.size()+offset; i++) {
			if constexpr (!withNewLines)  func(x, y, i);
			if (string[i] != '\n') {
				if constexpr (withNewLines)  func(x, y, i);
				x++;
			} else {
				x = 0;
				y++;
			}
		}
	}

	template<char alignX = 0, char alignY = 0> // values are [0, 100]
	void text(auto string, int startX = width*alignX/100, int startY = height*alignY/100, WORD color = defColor) {
		int offsetX = 0, offsetY = 0;
		if constexpr (alignX > 0 || alignY > 0) {
			int maxX = 0, maxY = 0;
			iterateText(string, [&](int x, int y, auto) {
				maxX = x > maxX ? x : maxX;
				maxY = y > maxY ? y : maxY;
			});
			offsetX = (int)(alignX/100.0 * maxX);
			offsetY = (int)(alignY/100.0 * maxY);
		}

		iterateText(string, [&](int x, int y, int i) {
			character(string[i], x + startX-offsetX, y + startY-offsetY, color);
		});
	}

	void text_Highlight(auto string, WORD color, int startX, int startY, int iStart, int iEnd) {
		iterateText<false,1>(string, [&](int x, int y, int i) {
			int tX = x +startX, tY = y +startY;
			if (iStart <= i&&i < iEnd && bounds(tX, tY)) {
				screenBuffer[tY * width + tX].Attributes = color;
			}
		});
	}

	void text_Cursor(auto string, int startX, int startY, int index, bool blink = true) {
		text_Highlight(string, 0x00F0, startX, startY, index, index+blink);
	}

	void text_CenterX(auto... A) { text<50 ,0>(A...); }
	void text_RightX (auto... A) { text<100,0>(A...); }



	// alt versions
	void text(wchar_t c, WORD col, int x, int y) { character(c, x, y, col); }
	void text(wchar_t* str, int x, int y, WORD col=defColor) { text(std::wstring(str), x, y, col); }
	// void text(auto str, WORD col) { text(str, auto, auto, color); }

}
#pragma once

#include "console.hh"
#include <string>

extern const WORD defColor = 0x000F;

// console Text class
struct ConsText {
	std::string str;
	std::vector<WORD> col;

	// add as many more operators as you please :D
	auto size      (auto... A) { auto V = str.size      (A...); update(); return V; }
	auto operator= (auto... A) { auto V = str.operator= (A...); update(); return V; }
	auto operator[](auto... A) { auto V = str.operator[](A...); update(); return V; }



	ConsText(std::string _str, char FG[]="", char BG[]="") {
		str = _str;
		update();
		fillCol(defColor);
		initCol(FG, BG);
	}
	ConsText() {}

	void fillCol(WORD color) {
		for (int i=0; i < col.size(); i++) {
			col[i] = color;
	} }
	// void initCol(char FG[]) { initCol(FG, ""); }
private:
	void update() {
		col.resize(str.size(), defColor);
	}

	void initCol(char FG[], char BG[]) {
		for (int i=0; i < str.size() && FG[i] != '\0'; i++) {
			char c = FG[i];
			unsigned N;
			/**/ if ('0' <= c&&c <= '9') { N = c - '0'; }
			else if ('A' <= c&&c <= 'F') { N = c - 'A' + 10; }
			else if ('a' <= c&&c <= 'f') { N = c - 'a' + 10; }
			else continue;
			col[i] &= 0xFFF0;
			col[i] |= N;
		}
		for (int i=0; str.size() && BG[i] != '\0'; i++) {
			char c = BG[i];
			unsigned N;
			/**/ if ('0' <= c&&c <= '9') { N = c - '0'; }
			else if ('A' <= c&&c <= 'F') { N = c - 'A' + 10; }
			else if ('a' <= c&&c <= 'f') { N = c - 'a' + 10; }
			else continue;
			col[i] &= 0xFF0F;
			col[i] |= N << 4;
		}
	}
};

struct Mask {
	// possibly add more functions here
	int x0, y0;
	int x1, y1;
	Mask(int _x0=0, int _y0=0, int _x1=~0u, int _y1=~0u) {
		x0 = _x0;  y0 = _y0;
		x1 = _x1;  y1 = _y1;
	}

	bool bounds(int x, int y) {
		return x0 <= x&&x < x1
		    && y0 <= y&&y < y1;
	}
};

namespace Draw {

	namespace { // anonymous
		ConsoleDisplay* ConsoleTarget;
		CHAR_INFO* screenBuffer;
		short width;
		short height;

		Mask windowMask = Mask{};
		Mask currentMask = windowMask;
		bool bounds(int x, int y) {
			return windowMask.bounds(x,y) && currentMask.bounds(x,y);
		}

		int currentWidth;


		std::wstring toWstring(std::string str) {
			return std::wstring(str.begin(), str.end());
		}

		bool hasElement(auto& vec, auto e) {
			return std::any_of(vec.begin(), vec.end(), [&](int i){ return i == e; });
		}

		int sizeStr(ConsText str)     { return str.size(); }
		int sizeStr(std::wstring str) { return str.size(); }
		int sizeStr(std::string str)  { return str.size(); }
		int sizeStr(wchar_t* str)     { return sizeStr(std::wstring(str)); }
		int sizeStr(char* str)        { return sizeStr(std::string(str));  }

		WORD getColor(ConsText str, int i, WORD def) { return str.col[i]; }
		WORD getColor(auto str, int i, WORD def)     { return def; }
	}

	void setMask(Mask &m) { currentMask = m; }
	void setMask(int x0, int y0, int x1, int y1) { currentMask = Mask{x0, y0, x1, y1}; }
	void returnMask() { currentMask = windowMask; }

	void setTextWidth(int w) { currentWidth = w; }
	void returnTextWidth() { currentWidth = width; }

	void setConsole(ConsoleDisplay* console) {
		ConsoleTarget = console;
		screenBuffer  = console->screenBuffer;
		width   = console->WIDTH;
		height  = console->HEIGHT;

		windowMask = Mask{0, 0, width, height};
		currentWidth = width;
		returnMask();
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

	void character(wchar_t c, int x, int y, WORD color=defColor) {
		if (!bounds(x,y)) { return; }
		if (c != L'\x7F') { screenBuffer[y * width + x].Char.UnicodeChar = c; }
		// /**/ if (color != 0xFFFF) { screenBuffer[y * width + x].Attributes = color; }
		// else if (color == 0x4000) { screenBuffer[y * width + x].Attributes ^= 0x4000; }
		switch (color) {
			default:     screenBuffer[y * width + x].Attributes = color;  break;
			case 0x4000: screenBuffer[y * width + x].Attributes ^= 0x4000;  break;
			case 0xFFFF: break;
		}
	}

	void rect(int startX, int startY, int endX, int endY, wchar_t charac=L'█', WORD color=defColor) {
		for (int y = startY; y < endY; y++) {
		for (int x = startX; x < endX; x++) {
			character(charac, x, y, color);
		} }
	}

	void lineX(int y, int startX=0, int endX=width, wchar_t charac=L'█', WORD color=defColor) {
		for (int x = startX; x < endX; x++) {
			character(charac, x, y, color);
		}
	}

	void lineY(int x, int startY=0, int endY=height, wchar_t charac=L'█', WORD color=defColor) {
		for (int y = startY; y < endY; y++) {
			character(charac, x, y, color);
		}
	}

	template<bool noBreak = false, bool withNewLines = false, int offset = 0>
	void iterateText(auto string, auto&& func) {
		if constexpr (noBreak) { return iterateTextNoBreak<withNewLines, offset>(string, func); }

		int x = 0, y = 0;
		for (int i=0; i < sizeStr(string)+offset; i++) {
			if constexpr (withNewLines)  func(x, y, i);
			if (string[i] != '\n') {
				if constexpr (!withNewLines)  func(x, y, i);
				x++;
			} else {
				x = 0;
				y++;
		} }
	}

	template<bool withNewLines = false, int offset = 0>
	void iterateTextNoBreak(auto string, auto&& func) {
		std::vector<int> breaks{-1};
		std::vector<int> enters{}; // required breaks
		for (int i=0; i < sizeStr(string); i++) {
			switch(string[i]) {
				case '\n':
					enters.push_back(i);
				case ' ':
					breaks.push_back(i);
					break;
		} }
		breaks.push_back(sizeStr(string));

		int total = breaks[1];
		for (int i=0; i < breaks.size()-1; i++) {
			int size = breaks[i+1] - breaks[i];
			if (size > currentWidth) {
				int mult = size/currentWidth;
				for (int j=1; j <= mult; j++) {
					breaks.insert(breaks.begin()+i, breaks[i] + currentWidth*j);
					i++;
				}
				size = breaks[i+1] - breaks[i];
			}
			// the line's allowed to continue
			if (total + size <= currentWidth && !hasElement(enters, breaks[i])) {
				total += size;
				breaks.erase(breaks.begin()+i);
				i--;
			} else { // the line doesn't fit!
				total = size;
			}
		}

		int x = 0, y = 0;
		for (int i=0; i < sizeStr(string)+offset; i++) {
			/**/ if constexpr (withNewLines) { func(x, y, i); }
			else if (string[i] != '\n')      { func(x, y, i); }
			if (!hasElement(breaks, i)) { // if not-break
				x++;
			} else {
				x = 0;
				y++;
		} }
	}

	template<bool noBreak = false>
	std::pair<int, int> getTextSize(auto string) {
		int maxX = 0, maxY = 0;
		iterateText<noBreak>(string, [&](int x, int y, auto) {
			maxX = std::max(x, maxX);
			maxY = std::max(y, maxY);
		});
		return {maxX+1, maxY+1};
	}

	template<char alignX = 0, char alignY = 0, bool noBreak = false> // align values are [0, 100]
	void text(auto string, int startX = width*alignX/100, int startY = height*alignY/100, WORD color = defColor) {
		auto [maxX, maxY] = getTextSize<noBreak>(string);
		int offsetX = (alignX/100.0 * (maxX-1));
		int offsetY = (alignY/100.0 * (maxY-1));

		iterateText<noBreak>(string, [&](int x, int y, int i) {
			WORD outColor = getColor(string, i , color);
			character(string[i], x + startX-offsetX, y + startY-offsetY, outColor);
		});
	}

	template<char alignX = 0, char alignY = 0>
	void textNoBreak(auto... A) {
		text<alignX,alignY,true>(A...);
	}

	template<bool noBreak = false>
	void text_Highlight(auto string, WORD color, int startX, int startY, int iStart, int iEnd) {
		iterateText<noBreak,false,1>(string, [&](int x, int y, int i) {
			int tX = x +startX, tY = y +startY;
			if (iStart <= i&&i < iEnd && bounds(tX, tY)) {
				if (color != 0xFFFF) { screenBuffer[tY * width + tX].Attributes = color; }
				else                 { screenBuffer[tY * width + tX].Attributes |= 0x4000; }
			}
		});
	}

	template<bool noBreak = false>
	void text_Cursor(auto string, int startX, int startY, int index, bool blink = true, WORD color=0x00F0) {
		text_Highlight<noBreak>(string, color, startX, startY, index, index+blink);
	}

	template<auto... A>
	void textNoColor(auto string, int x, int y) {
		text<A...>(string, x, y, 0xFFFF);
	}

	template<auto... A>
	std::pair<int,int> getIndexPos(auto string, int index) {
		int px, py;
		// int numWordBr = -1;
		iterateText<A...>(string, [&](int x, int y, int i) {
			// if (x==0 && y>0 && string[i-1] != '\n') { numWordBr++; }
			if (i == index) { px = x; py = y; }
		});
		return {px, py};
	}

	// void text_CenterX(auto... A) { text<50 ,0>(A...); }
	// void text_RightX (auto... A) { text<100,0>(A...); }



	// alt versions
	void lineX(int y, wchar_t c=L'█', WORD col=defColor)
	   { lineX(y, 0, width, c, col); } // TODO: figure out why this isn't working

	void lineY(int x, wchar_t c=L'█', WORD col=defColor)
	   { lineY(x, 0, height, c, col); }

	// void lineX(int y, wchar_t c=L'█')
	//    { lineX(y, 0, width, c, defColor); }
	// void lineX(int y, wchar_t c, WORD col)
	//    { lineX(y, 0, width, c, col); }

	// void lineY(int x, wchar_t c=L'█')
	//    { lineY(x, 0, height, c, defColor); }
	// void lineY(int x, wchar_t c, WORD col)
	//    { lineY(x, 0, height, c, col); }


	void text(wchar_t c, WORD col, int x, int y) { character(c, x, y, col); }
	void text(wchar_t* str, int x, int y, WORD col=defColor) { text(std::wstring(str), x, y, col); }
	// template<char X, char Y, bool B>
	// void text(auto str, WORD col) { text<X,Y,B>(str, width*X/100, height*Y/100, col); }
}


template<unsigned size = 128>
std::string format(auto... A) {
	char output[size];
	sprintf(output,/* size,*/ A...);
	return std::string(output);
}

template<unsigned size = 128>
std::wstring wformat(auto... A) {
	wchar_t output[size];
	swprintf(output,/* size,*/ A...);
	return std::wstring(output);
}
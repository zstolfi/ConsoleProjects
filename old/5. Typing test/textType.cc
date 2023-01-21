#include "console.hh"
#include "draw.hh"

#include <map>
#include <vector>

const short WIDTH  = 80;
const short HEIGHT = 20;
const int fontW = 12;
const int fontH = 24;

std::string textBuffer;
std::vector<int> lineLengths;

int cursorPos;
int cursorX;
int cursorY;

float keyRepeatRate;

unsigned char prevKey;
unsigned char lastKey;
float lastKeyTime;
float lastRepeatTime;
int   keyRepCounter;

// key code -> inserted char
std::map<unsigned char, char> keyMap;
std::map<unsigned char, char> keyMapShift;

class Game_Typing : public ConsoleDisplay {
	virtual void setup() {
		this->title = L"Typing Test";
		this->setFont(fontW, fontH);

		textBuffer = "";
		cursorPos = 0;
		cursorX = 0;
		cursorY = 0;

		keyRepeatRate = 30;

		prevKey = 0;
		lastKey = 0;
		lastKeyTime = 0;
		lastRepeatTime = 0;
		keyRepCounter = 0;

		lineLengths = {0};
		keyMap.clear();
		keyMapShift.clear();

		initKeyMap();
	}

	virtual void run() {
		if (keysDiff[VK_ESCAPE] == 1) {
			this->exit();
		}

		/* LOGIC */
		for (int i=0; i < 255; i++) {
			if (keysDiff[i] == 1) {
				typeShift(i, keys[VK_SHIFT]);
				lastKey = i;
				lastKeyTime = this->totalTime;
				lastRepeatTime = this->totalTime;
		} }

		if (lastKey != prevKey) {
			keyRepCounter = 0;
		}
		prevKey = lastKey;

		// repeated keys
		float timeSince = this->totalTime - lastKeyTime;
		if (timeSince >= 0.5 && keys[lastKey]) {
			// https://www.desmos.com/calculator/aprbziudt0
			int targetCount = (int)(keyRepeatRate * (timeSince - 0.5));
			if (targetCount > keyRepCounter) {
				keyRepCounter++;
				typeShift(lastKey, keys[VK_SHIFT]);
				lastRepeatTime = this->totalTime;
		} }

		/* DRAW STUFF */
		Draw::clearScreen();

		draw();
	}


	void typeShift(char c, bool shift = false, bool caps = false) {
		if (!shift) { typeAction(c); }
		else { typeActionShift(c); }
	}



	void getCursorXY() { // not working, discontinued
		int x = 0, y = 0;
		for (int i=0, total=0; i < lineLengths.size(); i++) {
			total += lineLengths[i];
			x = total + i - cursorPos;
			if (total >= cursorPos-i)  break;
			y++;
		}
		cursorX = x; cursorY = y;
	}

	void addChar(int index, char c) {
		textBuffer.insert(textBuffer.begin()+index, c);

		getCursorXY();

		if (c == '\n') {
			int curLineLen = lineLengths[cursorY];
			lineLengths[cursorY] = curLineLen-cursorX;
			lineLengths.insert( lineLengths.begin()+cursorY+1, cursorX);
		} else {
			lineLengths[cursorY]++;
		}
	}

	void removeChar(int index) {
		if (textBuffer.size() < 1) { return; }
		textBuffer.erase(index, 1);

		getCursorXY();

		// int affected = (textBuffer[index-1] == '\n') ? cursorY-1 : cursorY;
		int affected = cursorY;
		if (cursorX == 0 && cursorY > 0) {
			lineLengths[affected-1] = lineLengths[affected-1] + lineLengths[affected];
			lineLengths.erase(lineLengths.begin()+affected);
		} else {
			lineLengths[affected]--;
		}
	}

	void typeAction(char c) {
		if (keyMap.contains(c)) {
			addChar(cursorPos, keyMap[c]);
			cursorPos++;
		}

		switch(c) {
			case VK_BACK:
				if (cursorPos > 0) {
					removeChar(cursorPos-1);
					cursorPos--;
				}
				break;

			case VK_DELETE:
				removeChar(cursorPos);
				break;

			case VK_RETURN:
				// textBreaks.insert(
				// 	std::upper_bound( textBreaks.begin(), textBreaks.end(), cursorPos), cursorPos
				// );
				break;



			case VK_LEFT:
				if (cursorPos > 0) {
					cursorPos--;
				}
				break;

			case VK_RIGHT:
				if (cursorPos < textBuffer.size())
				cursorPos++;
				break;

			case VK_HOME:
				cursorPos = 0;
				break;

			case VK_END:
				cursorPos = textBuffer.size();
				break;



			case VK_F1 ... VK_F24:
				// functionKeyAction(c - VK_F1 + 1);
				break;

			default:
				break;
		}
	}

	void typeActionShift(char c) {
		if (keyMapShift.contains(c)) {
			addChar(cursorPos, keyMapShift[c]);
			cursorPos++;
		} else {
			switch(c) {
				/**/

				default: // if shift makes no difference
					typeAction(c);
					break;
			}
		}
	}

	void initKeyMap() {
		initRangeOffset(&keyMapShift, 'A', 'Z');
		initRangeOffset(&keyMap,      '0', '9');
		initRangeOffset(&keyMap,      'A', 'Z', 'a');
		initRangeOffset(&keyMap, VK_NUMPAD0, VK_NUMPAD9, '0');

		keyMap[VK_SPACE]  = ' ';
		keyMap[VK_RETURN] = '\n';
		keyMap[VK_TAB]    = '\t';

		keyMapShift['1'] = '!';
		keyMapShift['2'] = '@';
		keyMapShift['3'] = '#';
		keyMapShift['4'] = '$';
		keyMapShift['5'] = '%';
		keyMapShift['6'] = '^';
		keyMapShift['7'] = '&';
		keyMapShift['8'] = '*';
		keyMapShift['9'] = '(';
		keyMapShift['0'] = ')';

		keyMap[VK_OEM_1]      = ';';
		keyMap[VK_OEM_2]      = '/';
		keyMap[VK_OEM_3]      = '`';
		keyMap[VK_OEM_4]      = '[';
		keyMap[VK_OEM_5]      = '\\';
		keyMap[VK_OEM_6]      = ']';
		keyMap[VK_OEM_7]      = '\'';
		keyMap[VK_OEM_PLUS]   = '=';
		keyMap[VK_OEM_MINUS]  = '-';
		keyMap[VK_OEM_COMMA]  = ',';
		keyMap[VK_OEM_PERIOD] = '.';

		keyMap[VK_MULTIPLY]   = '*';
		keyMap[VK_ADD]        = '+';
		keyMap[VK_SUBTRACT]   = '-';
		keyMap[VK_DIVIDE]     = '/';
		keyMap[VK_DECIMAL]    = '.';

		keyMapShift[VK_OEM_1]      = ':';
		keyMapShift[VK_OEM_2]      = '?';
		keyMapShift[VK_OEM_3]      = '~';
		keyMapShift[VK_OEM_4]      = '{';
		keyMapShift[VK_OEM_5]      = '|';
		keyMapShift[VK_OEM_6]      = '}';
		keyMapShift[VK_OEM_7]      = '\"';
		keyMapShift[VK_OEM_PLUS]   = '+';
		keyMapShift[VK_OEM_MINUS]  = '_';
		keyMapShift[VK_OEM_COMMA]  = '<';
		keyMapShift[VK_OEM_PERIOD] = '>';
	}

	template <typename Map>
	void initRangeOffset(Map* M, int start, int end, int offsetTo) {
		for (int i = start; i <= end; i++) {
			(*M)[i] = i - start + offsetTo;
		}
	}
	template <typename Map>
	void initRangeOffset(Map* M, int s, int e) {
		initRangeOffset(M, s, e, s);
	}



	void draw() {
		bool blink = std::fmod(this->totalTime - lastRepeatTime, 1) < 0.5;
		// Draw::text(textBuffer + (blink ? "_" : " "), 0x000F, 0, 0);
		Draw::text(textBuffer);
		Draw::text_Cursor(textBuffer, 0, 0, cursorPos, blink);

		// getCursorXY();
		// wchar_t debugText[256];
		// swprintf_s(debugText, 256, L"%d\n%d", cursorX, cursorY);
		// Draw::text(debugText, WIDTH-10, HEIGHT-7);
		// for (int i=0; i < lineLengths.size(); i++) {
		// 	Draw::text<100,0>(std::to_wstring(lineLengths[i]), WIDTH-4, i);
		// }
	}
};

int main(int argc, char* argv[]) {
	Game_Typing game;
	game.initConsole(WIDTH, HEIGHT);
	Draw::setConsole(&game);

	game.start();

	return 0;
}
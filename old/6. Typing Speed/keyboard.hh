#pragma once

#include <map>

class KeyboardInput {
public:
	float repeatRate = 30;
	float blinkRate = 1;
	bool arrowControl;

	int cursorPos;

	bool pressed;

	KeyboardInput(bool _arrowControl=true) {
		arrowControl = _arrowControl;
		init();
	}

protected:
	unsigned char prevKey;
	unsigned char lastKey;
	float lastKeyTime;
	float lastRepeatTime;
	int   keyRepCounter;

	// key code -> inserted char
	std::map<unsigned char, char> keyMap;
	std::map<unsigned char, char> keyMapShift;

public:
	void init() {
		cursorPos = 0;
		pressed = false;

		// repeatRate = 30;
		// blinkRate = 1;

		prevKey = 0;
		lastKey = 0;
		lastKeyTime = 0;
		lastRepeatTime = 0;
		keyRepCounter = 0;

		keyMap.clear();
		keyMapShift.clear();

		initKeyMap();
	}

	void update(auto* game, std::string& textBuffer) {
		if (arrowControl) { // make sure the cursor isn't out of bounds
			cursorPos = std::min(cursorPos, (int)textBuffer.size());
		} else {
			cursorPos = textBuffer.size();
		}

		pressed = false;

		for (int i=0; i < 255; i++) {
			if (game->keysDiff[i] == 1) {
				keyType(textBuffer, i, game->keys[VK_SHIFT]);
				lastKey = i;
				lastKeyTime = game->totalTime;
				lastRepeatTime = game->totalTime;
		} }

		if (lastKey != prevKey) {
			keyRepCounter = 0;
		}
		prevKey = lastKey;

		// repeated keys
		float timeSince = game->totalTime - lastKeyTime;
		if (timeSince >= 0.5 && game->keys[lastKey]) {
			// https://www.desmos.com/calculator/aprbziudt0
			int targetCount = (int)(repeatRate * (timeSince - 0.5));
			if (targetCount > keyRepCounter) {
				keyRepCounter++;
				keyType(textBuffer, lastKey, game->keys[VK_SHIFT]);
				lastRepeatTime = game->totalTime;
		} }
	}

	bool getBlink(float totalTime) {
		return std::fmod(totalTime - lastRepeatTime, 1/blinkRate) < 0.5/blinkRate;
	}
	bool getBlink(auto* game) {
		return getBlink(game->totalTime);
	}

protected:
	void keyType(std::string& textBuffer, char c, bool shift = false, bool caps = false) {
		if (!shift) { typeAction(textBuffer, c); }
		else { typeActionShift(textBuffer, c); }
		pressed = true;
	}

	void addChar(std::string& textBuffer, int index, char c) {
		textBuffer.insert(textBuffer.begin()+index, c);
	}

	void removeChar(std::string& textBuffer, int index) {
		if (textBuffer.size() < 1) { return; }
		textBuffer.erase(index, 1);
	}

	void typeAction(std::string& textBuffer, char c) {
		if (keyMap.contains(c)) {
			addChar(textBuffer, cursorPos, keyMap[c]);
			cursorPos++;
		}

		switch(c) {
			case VK_BACK:
				if (cursorPos > 0) {
					removeChar(textBuffer, cursorPos-1);
					cursorPos--;
				}
				break;

			case VK_DELETE:
				removeChar(textBuffer, cursorPos);
				break;



			case VK_F1 ... VK_F24:
				// functionKeyAction(c - VK_F1 + 1);
				break;

			default:
				break;
		}

		if (arrowControl) {
			switch(c) {
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

				default:
					break;
			}
		}

	}

	void typeActionShift(std::string& textBuffer, char c) {
		if (keyMapShift.contains(c)) {
			addChar(textBuffer, cursorPos, keyMapShift[c]);
			cursorPos++;
		} else {
			switch(c) {
				/**/

				default: // if shift makes no difference
					typeAction(textBuffer, c);
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
};
#include "console.hh"
#include "draw.hh"
#include "keyboard.hh"
#include "consts.hh"

#include <fstream>
#include <sstream>
#include <map>
#include <vector>

const short WIDTH  = 80;
const short HEIGHT = 25;
int fontW = 12;
int fontH = 24;

std::string textFileName = "src/excerpt.txt";
GameModeType gameMode;

ConsText goalText;
std::vector<std::string> words;
std::vector<int> wordStarts;
std::string wordInput;
KeyboardInput keyInput;

bool misspell;
int  misspellPos;
std::map<int, bool> mistakes;

float timeStart;
float timeEnd;

int currentWord;
int numCorrect;
int position;
int currentY;

int goalWindowSize;
int goalTextSize;
ConsText textBox;
Mask goalMask;
int scrollPos;
int maxScroll;

int carPos;
bool scrollBar;
int scrollBarSize;
bool autoScroll;
float autoScrollTimeStart;


class Game_Typing : public ConsoleDisplay {
	virtual void setup() {
		this->title = L"Typing Test";
		// this->setFont(fontW, fontH);

		std::ifstream readFile{ textFileName };
		if (!readFile) {
			this->error("Unable to read file: \"" + textFileName + "\"");
		}		
		std::stringstream tempBuffer;
		tempBuffer << readFile.rdbuf();
		goalText = tempBuffer.str();
		words = splitIntoWords(goalText.str);
		for (int i=0, total=0; i < words.size(); i++) {
			wordStarts.push_back(total);
			total += words[i].size() + 1;
		}

		gameMode = TYPING;
		misspell = false;
		misspellPos = 0;
		mistakes.clear();

		currentWord = 0;
		numCorrect = 0;
		position = 0;
		currentY = 0;

		timeStart = 0;
		timeEnd = 0;

		wordInput = "";
		keyInput.arrowControl = false;
		keyInput.blinkRate = 2;
		scrollPos = 0;
		carPos = -5;

		autoScroll = false;
		autoScrollTimeStart = this->totalTime;

		// grahpics
		goalWindowSize = 7;

		initSprites();

		std::tie(std::ignore , goalTextSize) = Draw::getTextSize<true>(goalText);
		scrollBar = goalTextSize > goalWindowSize;
		int maxSize = goalWindowSize-2;
		scrollBarSize = std::clamp(maxSize*goalWindowSize/goalTextSize, 1, maxSize-1);

		maxScroll = goalTextSize-goalWindowSize;
		if (!scrollBar) { maxScroll = 0; }
		
		textBox = "[" + std::string(scrollBar ? WIDTH-3 : WIDTH-2, ' ') + "]";
		goalMask = {0, HEIGHT-goalWindowSize-2, WIDTH, HEIGHT-2};
	}

	virtual void run() {
		if (keysDiff[VK_ESCAPE] == 1) {
			this->exit();
		}
		Draw::clearScreen();

		/* LOGIC */
		switch (gameMode) { 
			case TYPING: {
				keyInput.update(this, wordInput);
				// if (!keyInput.pressed) { break; }

				// process input
				std::string word = words[currentWord];
				if (wordInput.size() > word.size()+10) { wordInput.resize(word.size()+10); }

				int wordStart = wordStarts[currentWord];
				int wordEnd = wordStart + word.size();
				int progress = wordVerify();
				numCorrect = wordStart + progress;
				position = wordStart + wordInput.size();
				if (timeStart == 0 && progress > 0) { timeStart = this->totalTime; }

				if (numCorrect == goalText.size()) {
					gameMode = FINISHED;
					misspell = false;
					timeEnd = this->totalTime;
					break;
				}

				misspell = numCorrect < position;

				char lastChar = goalText[wordStart + word.size()];
				if (progress == wordInput.size()-1 && progress == word.size()
				&& wordInput.back() == lastChar) {
					currentWord++;
					misspell = false;
					wordInput = "";
				}
				if (misspell) { mistakes[numCorrect] = true; }

				if (!misspell) {
					// (sets the cursor's Y pos)
					// Very glitchy, but good enough for now. As long as
					// it's about on the right line it should be good
					std::tie(std::ignore , currentY) = Draw::getIndexPos<true,true,1>(goalText, position);
				}

				
				// handle scroll
				// if (keysDiff[VK_UP]   == 1) { scrollPos = std::max(scrollPos-1, 0); }
				// if (keysDiff[VK_DOWN] == 1) { scrollPos = std::min(scrollPos+1, maxScroll); }

				if (!autoScroll && scrollPos < maxScroll
				    && currentY - scrollPos > goalWindowSize/2) { autoScrollStart(); }

				if (autoScroll && this->totalTime - autoScrollTimeStart > 1.25) {
					autoScroll = false;
					stepLine();
				}
			} break;

			case FINISHED: {
				if (keysDiff[VK_BACK] == 1) {
					setup();
				}

				if (keysDiff[VK_UP]   == 1) { scrollPos = std::max(scrollPos-1, 0); }
				if (keysDiff[VK_DOWN] == 1) { scrollPos = std::min(scrollPos+1, maxScroll); }
			} break;

			case MENU: {

			} break;
		}

		/* DRAW STUFF */
		draw();
	}

	void autoScrollStart() {
		if (autoScroll) { stepLine(); }
		autoScroll = true;  autoScrollTimeStart = this->totalTime;
	}
	void stepLine() { scrollPos = std::min(scrollPos+1, maxScroll); }

	// tells us how many letters of the current word, are typed correctly
	int wordVerify() {
		int total = 0;
		std::string word = words[currentWord];
		for (int i=0; i < wordInput.size(); i++) {
			if (word[i] == wordInput[i]) {
				total++;
			} else {
				break;
			}
		}
		return total;
	}

	std::vector<std::string> splitIntoWords(std::string string) {
		std::vector<std::string> output;
		std::string word = "";
		for (int i=0; i < string.size(); i++) {
			switch (string[i]) {
				case '\n':
				case ' ':
					output.push_back(word);
					word = "";
					break;

				default:
					word += string[i];
					break;
		} }
		output.push_back(word);
		return output;
	}



	void draw() {
		bool blink = keyInput.getBlink(this);
		int W = scrollBar ? WIDTH-1 : WIDTH;

		/* TITLE */
		Draw::lineX(0, L'═');
		Draw::character(L'╒', 0,       0);
		Draw::character(L'╕', WIDTH-1, 0);
		Draw::text<50,0>(L" Alsin's EPIC Console Typing Game ", WIDTH/2, 0);

		/* BOTTOM TEXT */
		Draw::lineX(HEIGHT-goalWindowSize-3, L'═');
		Draw::lineX(HEIGHT-2, 0, W, L'─', 0x0008);
		for (int i=0; i < goalText.size(); i++) {
			goalText.col[i] = i < numCorrect ? 0x000A : 0x0008;
		}
		if (misspell) {
			for (int i = numCorrect; i < position; i++) {
				goalText.col[i] = 0x00C7;
		} }

		/* TYPING TEXT */
		int divideY = HEIGHT-goalWindowSize-2;
		Draw::setTextWidth(W);
		Draw::setMask(goalMask);

		Draw::textNoBreak      (goalText, 0, divideY-scrollPos);
		Draw::text_Cursor<true>(goalText, 0, divideY-scrollPos, position, true, 0x0070);

		Draw::returnTextWidth();
		Draw::returnMask();

		// draw scroll bar
		if (scrollBar) {
			int barStartY = std::round((float)scrollPos/maxScroll
			                * (goalWindowSize - scrollBarSize));
			// int barStartY = 2;

			Draw::character(L'▲', WIDTH-1, divideY,  0x0080);
			Draw::character(L'▼', WIDTH-1, HEIGHT-1, 0x0080);
			Draw::lineY(WIDTH-1, divideY+1, HEIGHT-1, L'█', 0x0008);
			Draw::lineY(WIDTH-1, divideY+1+barStartY, divideY+1 +barStartY+scrollBarSize, L'█', 0x0007);
		}

		/* RACECAR */
		int carWidth = 30;
		carPos = std::max<int>(carPos, -5 + (WIDTH-carWidth) * position/goalText.size());
		Draw::text(sprite["car"], carPos, divideY-7);

		switch (gameMode) { 
			case TYPING: {

				textBox.fillCol(!misspell ? 0x0007 : 0x00C7);
				textBox.col.front() = 0x0008;
				textBox.col.back()  = 0x0008;

				Draw::text(textBox, 0, HEIGHT-1);
				Draw::textNoColor(wordInput, 1, HEIGHT-1);
				Draw::text_Cursor(wordInput, 1, HEIGHT-1, keyInput.cursorPos, blink/*, 0xFFFF*/);
			} break;

			case FINISHED: {
				Draw::text<50,0>("  TYPING COMPLETED! :D  ", WIDTH/2, 2, 0x00A0);

				/* stats */
				float duration = timeEnd - timeStart;
				int WPM = 0.2*goalText.size() / (duration/60);
				float accuracy = 1 - (float)mistakes.size()/goalText.size();

				Draw::text<0,0>(format("Words Per Minute: %d", WPM), 5, 4);

				int ACC = 1000*accuracy;
				Draw::text<100,0>(format("Accuracy: %d.%d%%", ACC/10, ACC%10), WIDTH-6, 4);

			} break;

			case MENU: {

			} break;
		}

		float diff = this->totalTime - autoScrollTimeStart;
		bool scrollBlink = autoScroll && std::fmod(diff, 0.5) < 0.25;
		if (scrollBlink) { Draw::lineX(HEIGHT-1, 0, W, L'\x7F', 0x0080); }

		// debug text
		// Draw::text<100,0>(format("currentY: %d",      currentY),      WIDTH-1, 5);
		// Draw::text<100,0>(format("Time Diff: %f", this->totalTime - autoScrollTimeStart),     WIDTH-1, 6);
	}
};

void handleArguments(int argc, char* argv[]) {
	if (argc > 1) { textFileName = argv[1]; }
}

int main(int argc, char* argv[]) {
	handleArguments(argc, argv);
	Game_Typing game;
	game.initConsole(WIDTH, HEIGHT, fontW, fontH);
	Draw::setConsole(&game);

	game.start();

	return 0;
}
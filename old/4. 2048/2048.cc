#include "console.hh"
#include "draw.hh"
#include "consts.hh"
#include <algorithm>

const short WIDTH  = 33;
const short HEIGHT = 19;

int board[4][4] = { 0 };
int score;
int numMoves;
bool gameOver;

class Game_2048 : public ConsoleDisplay {
	virtual void setup() {
		this->title = L"2048";

		memset(board, 0, sizeof(int) * 4*4);
		score = 0;
		numMoves = 0;
		gameOver = false;
	}

	void endGame() {
		gameOver = true;
	}

	virtual void run() {
		if (gameOver && keysDiff[VK_BACK]) {
			setup();
		}

		/* GAMEPLAY */
		if (numMoves > 0) {
			Dir input;
			/**/ if (keysDiff['W'] == 1 || keysDiff[VK_UP]    == 1) { input = UP   ; }
			else if (keysDiff['S'] == 1 || keysDiff[VK_DOWN]  == 1) { input = DOWN ; }
			else if (keysDiff['A'] == 1 || keysDiff[VK_LEFT]  == 1) { input = LEFT ; }
			else if (keysDiff['D'] == 1 || keysDiff[VK_RIGHT] == 1) { input = RIGHT; }
			else { return; } // no input

			// directon, & if it's a legal move
			std::pair<Dir, bool> futureMoves[4];

			futureMoves[UP   ] = {UP    , shiftGrid(UP   ).first > 0};
			futureMoves[DOWN ] = {DOWN  , shiftGrid(DOWN ).first > 0};
			futureMoves[LEFT ] = {LEFT  , shiftGrid(LEFT ).first > 0};
			futureMoves[RIGHT] = {RIGHT , shiftGrid(RIGHT).first > 0};

			bool movesLeft = false;
			for (int d=0; d < 4; d++) {
				movesLeft  |= futureMoves[d].second;
			}
			// no moves left
			if (!movesLeft) { endGame(); }
			// current move is legal
			if (futureMoves[input].second) {

		
				auto [ _ , addedScore] = shiftGrid(input, true);
				score += addedScore;

				cellSet(randomOpen(), newNumber());
				numMoves++;
			}
		} else { // first move!
			cellSet(randomOpen(), newNumber());
			cellSet(randomOpen(), newNumber());
			numMoves++;
		}

		/* DRAW STUFF */
		Draw::clearScreen();
		draw();
	}



	std::pair<int, int> shiftGrid(Dir direction, bool updateMain = false) {
		auto lines = directionTable[direction];

		int copies[4][4];
		for (int i=0; i < 4; i++) {
		for (int j=0; j < 4; j++) {
			auto [y,x] = lines[i][j];
			copies[i][j] = board[y][x];
		} }

		int affectedLines = 0;
		int addedScore = 0;
		for (int i=0; i < 4; i++) {
			auto [affected, lineScore] = shiftMerge(copies[i]);
			affectedLines += affected;
			addedScore += lineScore;
		}

		if (updateMain) {
			for (int i=0; i < 4; i++) {
			for (int j=0; j < 4; j++) {
				auto [y,x] = lines[i][j];
				board[i][j] = copies[y][x];
			} }
		}

		return {affectedLines, addedScore};
	}

	std::pair<bool, int> shiftMerge(int line[4]) {
		bool a = shiftLine(line);
		int  b = mergeLine(line);
		bool c = shiftLine(line);
		// return if anything happened, & score
		return {a || c || b>0, b};
	}

	bool shiftLine(int line[4]) {
		bool changed = false;
		std::vector<int> noZeros;
		for (int i=0; i < 4; i++) {
			if (line[i] != 0) {
				noZeros.push_back(line[i]);
		} }
		for (int i=0; i < 4; i++) {
			int newVal = (i < noZeros.size()) ? noZeros[i] : 0;
			if (line[i] != newVal) { changed = true; }
			line[i] = newVal;
		}
		// return if shift did anything
		return changed;
	}

	int mergeLine(int line[4]) {
		int lineScore = 0;
		for (int i=0; i < 4-1; i++) {
			if (line[i] == line[i+1] != 0) {
				line[i]   = 2*line[i];
				line[i+1] = 0;
				lineScore += line[i];
		} }
		// return score
		return lineScore;
	}

	int newNumber() {
		int random = rand();
		return (random  < 0.5*RAND_MAX) ? 2 : 4;
	}

	void cellSet(Pos cell, int val) {
		auto [y,x] = cell;
		board[y][x] = val;
	}

	Pos randomOpen() {
		std::vector<Pos> available;
		for (int y=0; y < 4; y++) {
		for (int x=0; x < 4; x++) {
			if (board[y][x] == 0) {
				available.push_back({y,x});
			}
		} }
		int index = rand() % available.size();
		return available[index]; // doesn't check when size() == 0
	}



	void draw() {
		std::wstring scoreStr = std::to_wstring(score);
		if (gameOver) {
			Draw::text_CenterX(L"Game Over! Score: " + scoreStr, 0x000F, WIDTH/2, HEIGHT-1);
		} else {
			// Draw::text_RightX(scoreStr,  0x0007, WIDTH-2, 0);
			// Draw::text_RightX(L"Score:", 0x0007, WIDTH-10, 0);
			Draw::text_CenterX(L"Score: " + scoreStr, 0x0007, WIDTH/2, 0);
		}


		std::wstring output = boardTemplate;

		for (int y=0; y < 4; y++) {
		for (int x=0; x < 4; x++) {
			int number = board[y][x];
			std::wstring formatted = std::to_wstring(number);
			if (number == 0) { formatted = L" "; }
			int padding = 6 - formatted.size();
			int padR = padding/2;
			int padL = padding - padR;

			formatted = std::wstring(padL, L' ') + formatted + std::wstring(padR, L' '); 

			std::wstring search = std::to_wstring(y) + std::to_wstring(x) + L"~~~~";
			
			int index = output.find(search);
			output.replace(index, 6, formatted);
		} }

		Draw::text(output, 0x000F, 2, 1);
	}
};

int main(int argc, char* argv[]) {
	Game_2048 game;
	game.initConsole(WIDTH, HEIGHT, 16, 32);
	Draw::setConsole(&game);

	game.start();

	return 0;
}
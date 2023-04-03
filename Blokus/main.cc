#include "console.hh"
#include "draw.hh"

#include "common.hh"
#include "symmetry.hh"
#include "matrix.hh"
#include "pieces.hh"
#include "board.hh"
// #include "board parse.hh"

#include <set>
#include <vector>
#include <array>

/* -------------- */
/*  App Behavior  */
/* -------------- */

class MainApp : public ConsoleWindow {
public: /* Initialize Canvas */
	const unsigned width = getWidth()/2, height = getHeight();
	using screenType = std::remove_reference_t<decltype(std::declval<MainApp&>().getScreenBuffer())>;
	using canvasType = DrawSquares<screenType, WCHAR, WORD>;

	canvasType canvas{
		getScreenBuffer(), getWidth(), getHeight() ,
		[](CHAR_INFO& pix, WCHAR chr) { pix.Char.UnicodeChar = chr; } ,
		[](CHAR_INFO& pix, WORD  col) { pix.Attributes       = col; }
	};

	/* Constructor */
	using ConsoleWindow::ConsoleWindow;

private: /* Game Variables */

	enum gameState {
		// https://files.catbox.moe/6x9rk6.png
		MENU ,
		ABOUT ,
		START_OPTIONS ,

		VIEW_BOARD ,

		PIECE_SELECT ,
		PIECE_MOVE ,
		POSITION_ACCEPT ,
		POSITION_REJECT ,

		PLAYER_FINAL_MOVE ,
		GAME_OVER ,
		STATS ,
		QUIT
	} state = MENU, prevState = MENU;

	seconds stateStartTime = totalTime;
	seconds stateTime = seconds{0}; // time since last state change, used for animation
	bool stateFirst = true; // first frame of a state
	void setState(gameState nextState) {
		prevState = state; state = nextState;
		stateStartTime = totalTime;
	}
	void undoState() { // only used in QUIT state
		state = prevState; prevState = MENU;
		stateStartTime = totalTime;
	}



	const std::array<WORD,4> colors {
		0x000C , /*RED*/
		0x000E , /*YELLOW*/
		0x000A , /*GREEN*/
		0x0009   /*BLUE*/
	};

	struct GameOptions {
		Size boardSize;
		std::array<int,4> corners;
		unsigned firstPlayer;
		// TODO: make proper constructor
		GameOptions() { boardSize = {0,0}; corners.fill(-1); firstPlayer = 0; }
	};

	struct Player {
		unsigned ID;
		unsigned color; // index to color palette
		unsigned corner;
		std::set<unsigned> pieces;

		explicit Player(unsigned ID, unsigned pos): ID{ID}, corner{pos} {
			for (std::size_t i=0; i < Pieces.size(); i++) {
				pieces.insert(i);
			}
		}
	};

	GameOptions gameOptions;
	std::vector<Player> players {};
	Board board {{20, 20}};


	int cursorX = 0;
	int cursorY = 0;
	unsigned selectedNum = 0;
	unsigned selectedRot = 0;
	PieceOption selected = Pieces[0].getOption(0);
	void updateSelection() { selected = Pieces[selectedNum].getOption(selectedRot); }

	virtual void setup() final {
		canvas.blankPix = CHAR_INFO{L' ', 0x000F};
		canvas.clear();
		// noDraw = true;
	}

	virtual void update() final {
		#define If_Key(VK,ACTION) if (keys[VK] == keyState::PRESSED) { ACTION }
		#define Key_To_State(VK,STATE) If_Key(VK, setState(STATE); return; )
		if (state != QUIT) { Key_To_State(VK_ESCAPE, QUIT); }
		if (state != prevState) { stateFirst = true; }
		stateTime = totalTime - stateStartTime;

		switch (state) {

		case MENU: { /* Display awesome logo, user can select 'about' or 'start options' */

			Key_To_State('A', ABOUT);
			Key_To_State('S', START_OPTIONS);
			} break;

		case ABOUT: { /* Info about the game, user can select 'back' */

			Key_To_State('B', MENU);
			} break;

		case START_OPTIONS: { /* Select colors, player count, player order */

			if (stateFirst) { gameOptions = GameOptions{}; }
			gameOptions.boardSize.m = 20;
			gameOptions.boardSize.n = 20;
			const int EMPTY = -1;
			gameOptions.corners[0] = 3;
			gameOptions.corners[1] = EMPTY;
			gameOptions.corners[2] = 0;
			gameOptions.corners[3] = 1;
			gameOptions.firstPlayer = 2;

			/*back*/
			Key_To_State('B', MENU);
			/*continue*/
			bool setupDone = false;
			If_Key('C', setupDone = true; setState(PIECE_SELECT); );
			if (setupDone) {
				/* initialize 'players' vector */
				for (unsigned i=0; i < 4; i++) {
					unsigned j = (i+gameOptions.firstPlayer) % 4; // move clockwise
					if (gameOptions.corners[j] == EMPTY) { continue; }
					players.emplace_back(players.size(), j);
				}
			}

			} break;

		case VIEW_BOARD: { /* Use arrow keys to view board history */

			} break;

		case PIECE_SELECT: { /* Select colors, player count, player order */

			if (stateFirst) { selectedRot = 0; }

			/*move selection*/
			int step = 0;
			If_Key(VK_LEFT , step--; );
			If_Key(VK_RIGHT, step++; );
			selectedNum += step + Pieces.size();
			selectedNum %= Pieces.size();
			updateSelection();

			Key_To_State(VK_RETURN, PIECE_MOVE);
			} break;

		case PIECE_MOVE: { /* Move selected piece with arrow keys, enter to place */

			#define Apply_Move(KEY,ACTION) if ((frameCount % 10 == 0) && keys[KEY] == keyState::HELD) { ACTION }
			Apply_Move(VK_LEFT , cursorX--;);
			Apply_Move(VK_RIGHT, cursorX++;);
			Apply_Move(VK_UP   , cursorY--;);
			Apply_Move(VK_DOWN , cursorY++;);
			#undef Apply_Move

			const Size& currentSize = selected.getShape().size();
			signed loopX0 = 0-currentSize.n, loopX1 = width-1;
			signed loopY0 = 0-currentSize.m, loopY1 = height-1;

			if (cursorX < loopX0) { cursorX = loopX1; }
			if (cursorX > loopX1) { cursorX = loopX0; }
			if (cursorY < loopY0) { cursorY = loopY1; }
			if (cursorY > loopY1) { cursorY = loopY0; }

			Key_To_State('B', PIECE_SELECT);
			// TODO: use more natural movements (i.e., flip rotate)
			int step = 0;
			If_Key(VK_OEM_COMMA , step--; );
			If_Key(VK_OEM_PERIOD, step++; );
			selectedRot += step + Pieces[selectedNum].numOptions();
			selectedRot %= Pieces[selectedNum].numOptions();
			updateSelection();

			Key_To_State(VK_RETURN, true ? POSITION_ACCEPT : POSITION_REJECT);
			} break;

		case POSITION_REJECT: { /* Blink for ~1 second */

			if (stateTime < seconds{0.5}) { break; }
			setState(PIECE_MOVE);
			} break;

		case POSITION_ACCEPT: { /* Blink for ~1 second, then go to next player */

			if (stateTime < seconds{0.5}) { break; }
			/*if currentPlayer.numPieces = 0*/
			/*or currentPlayer.numMovesAvailable = 0*/
				/*remove current player*/
				/*setState(PLAYER_FINAL_MOVE);*/
			/*else*/
				/*nextPlayer()*/

			setState(PIECE_SELECT);
			} break;

		case PLAYER_FINAL_MOVE: { /* display 'player X is out' message, then go to next player */

			if (stateTime < seconds{2}) { break; }
			/*if all players.numPieces = 0*/
				/*setState(GAME_OVER)*/
			/*else*/
				/*nextPlayer()*/
			setState(PIECE_SELECT);
			} break;

		case GAME_OVER: { /* display 'player X won' or possible ties */

			/*continue*/
			Key_To_State(VK_RETURN, STATS);
			} break;

		case STATS: { /* display stats for each player  + board history*/

			/*back to menu*/
			Key_To_State('M', MENU);
			/*quit*/
			Key_To_State('Q', QUIT);
			} break;

		case QUIT: {

			If_Key(VK_ESCAPE, quitConsole(); break; );
			If_Key('B', undoState(); break; );
			} break;
		}

		stateFirst = false;
		#undef If_Key
		#undef Key_To_State
	}

	virtual void draw() final {
		if (state == QUIT) {
			canvas.text(0, 0, L"Are you sure you want to quit?");
			canvas.text(0, 39, L"[ESC quit]      [B back]");
			return; // don't clear screen
		}
		canvas.clear();

		/* Draw the board */
		if (Is_Either(state, VIEW_BOARD, PIECE_SELECT, PIECE_MOVE ,
		              POSITION_ACCEPT, POSITION_REJECT ,
		              PLAYER_FINAL_MOVE, GAME_OVER)) {
			canvas.squareBorder({ 9, 9,22,22}, CHAR_INFO{L'█', 0x000F});
			// canvas.drawImplicit({10,10,20,20}, CHAR_INFO{L'█', 0x0008} ,
			// 	[&](signed x, signed y) {
			// 		return (x&1) ^ (y&1);
			// 	});
			canvas.drawImage({10,10,20,20}, board.render() ,
				[&](auto tile) {
					if (0 <= tile&&tile < 4) { return CHAR_INFO{L'█', colors[tile]}; }
					else { return CHAR_INFO{L'█', 0x0000}; }
				});
		}

		switch (state) {

		case MENU: {
			canvas.text(0, 0, L"<BLOKUS>");
			canvas.text(0, 39, L"[S start]      [A about]");
			} break;

		case ABOUT: {
			canvas.text(0, 0, L"This game was created by a really awesome dude.");
			canvas.text(0, 39, L"[B back]");
			} break;

		case START_OPTIONS: {
			canvas.text(0, 0, L"There are no options right now, press C to continue ");
			canvas.text(0, 39, L"[C continue]      [B back]");
			} break;

		case PIECE_SELECT: {

			const Size& currentSize = selected.getShape().size();
			canvasType::bounds pieceBounds = {2, 2, currentSize.n, currentSize.m};
			canvas.drawImplicit(pieceBounds, CHAR_INFO{L'█', 0x0004} ,
				[&](signed x, signed y) { return selected.getShape()[y, x]; }
			);

			canvas.text(0, 0, L"Arrow keys to select piece type");
			canvas.text(0, 39, L"[ENTER continue]");
		} break;

		case PIECE_MOVE: {
			canvas.text(0, 0, L"Move the piece around & choose permutation with '<' or '>'");
			canvas.text(0, 39, L"[B back]      [ENTER continue]");

			const Size& currentSize = selected.getShape().size();
			canvasType::bounds pieceBounds = {cursorX, cursorY, currentSize.n, currentSize.m};
			canvas.drawImplicit(pieceBounds, CHAR_INFO{L'█', 0x0004} ,
				[&](signed x, signed y) { return selected.getShape()[y, x]; });
			} break;

		case POSITION_ACCEPT: {
			canvas.text(0, 0, L"Position accepted!");
			} break;

		case POSITION_REJECT: {
			canvas.text(0, 0, L"Position REJECTED!");
			} break;

		case PLAYER_FINAL_MOVE: {} break;

		case GAME_OVER: {
			canvas.text(0, 0, L"GAME OVER");
			canvas.text(0, 39, L"[ENTER view stats]");
			} break;

		case STATS: {
			canvas.text(0, 0, L"(Stats go here, but nothing yet)");
			canvas.text(0, 39, L"[M menu]      [Q quit]");
			} break;

		/* Do nothing for these */
		case VIEW_BOARD: {} break;
		case QUIT: {} break;
		}
	}
};



/* -------------- */
/*      Main      */
/* -------------- */

#include "board parse.hh"
#include <fstream>

struct parseArguments {
	bool displayMode = false;
	std::stringstream historyStr;

	parseArguments(int argc, char* argv[]) {
		if (arvc > 1) {
			displayMode = true;
			if (auto file = std::ifstream{args[1]}) {
				historyStr << file.rdbuf();
			} else {
				std::cerr << "Error!: Failed to open \"" << args[1] << "\"\n";
				displayMode = false;
			}
		}
	}
};

int main(int argc, char* argv[]) {
	auto settings = parseArguments{argc, argv};
	MainApp app{L"Blokus Experiments", settings,
               {.width = 80, .height = 40,
                .fontW = 8 , .fontH  = 16}};

    app.start();
    while (!app.quit()) {
    	app.run();
    }

	return 0;
}
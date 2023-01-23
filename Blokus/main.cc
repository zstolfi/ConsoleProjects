#include "console.hh"
#include "draw.hh"

#include "common.hh"
#include "symmetry.hh"
#include "matrix.hh"
#include "pieces.hh"

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

	enum gameState { // https://files.catbox.moe/6x9rk6.png
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

	seconds stateTime = totalTime; // time since last state change, used for animation
	void setState(gameState nextState) {
		prevState = state; state = nextState;
		stateTime = totalTime;
	}
	void undoState() { // only used in QUIT state
		state = prevState; prevState = MENU;
		stateTime = totalTime;
	}



	int cursorX = 0;
	int cursorY = 0;
	PieceOption selected = Pieces[13].getOption(0);
	Size currentSize = selected.getShape().size();

	virtual void setup() final {
		canvas.blankPix = CHAR_INFO{L' ', 0x000F};
		canvas.clear();
		// noDraw = true;
	}

	virtual void update() final {
		if (state != QUIT)
			if (keys[VK_ESCAPE] == keyState::PRESSED) { setState(QUIT); return; }

		switch (state) {

		case MENU: { /* Display awesome logo, user can select 'about' or 'start options' */

			if (keys['A'] == keyState::PRESSED) { setState(ABOUT); break; }
			if (keys['S'] == keyState::PRESSED) { setState(START_OPTIONS); break; }
			} break;

		case ABOUT: { /* Info about the game, user can select 'back' */

			if (keys['B'] == keyState::PRESSED) { setState(MENU); break; }
			} break;

		case START_OPTIONS: { /* Select colors, player count, player order */

			/*continue*/
			if (keys['C'] == keyState::PRESSED) { setState(PIECE_SELECT); break; }
			/*back*/
			if (keys['B'] == keyState::PRESSED) { setState(MENU); break; }
			} break;

		case VIEW_BOARD: { /* Use arrow keys to view board history */

			} break;

		case PIECE_SELECT: { /* Select colors, player count, player order */

			/*move selection*/
			if (keys[VK_LEFT ] == keyState::PRESSED) { /* code */ }
			if (keys[VK_RIGHT] == keyState::PRESSED) { /* code */ }

			if (keys[VK_RETURN] == keyState::PRESSED) { setState(PIECE_MOVE); break; }
			} break;

		case PIECE_MOVE: { /* Move selected piece with arrow keys, enter to place */

			#define Apply_Move(KEY,ACTION) if ((frameCount % 10 == 0) && keys[KEY] == keyState::HELD) { ACTION }
			Apply_Move(VK_LEFT , cursorX--;)
			Apply_Move(VK_RIGHT, cursorX++;)
			Apply_Move(VK_UP   , cursorY--;)
			Apply_Move(VK_DOWN , cursorY++;)
			#undef Apply_Move

			signed loopX0 = 0-currentSize.n, loopX1 = width-1;
			signed loopY0 = 0-currentSize.m, loopY1 = height-1;

			if (cursorX < loopX0) { cursorX = loopX1; }
			if (cursorX > loopX1) { cursorX = loopX0; }
			if (cursorY < loopY0) { cursorY = loopY1; }
			if (cursorY > loopY1) { cursorY = loopY0; }

			// TODO: allow player to go back & select other piece
			// TODO: use more natural movements (i.e., flip rotate)
			// if (keys[VK_RETURN] == keyState::PRESSED) { 
			// 	setState((/*check piece placement*/) ? POSITION_ACCEPT : POSITION_REJECT); break;
			// }
			if (keys[VK_RETURN] == keyState::PRESSED) { setState(GAME_OVER); break; }
			} break;

		case POSITION_REJECT: { /* Blink for ~1 second */

			/*timeout code*/
			setState(PIECE_MOVE); break;
			} break;

		case POSITION_ACCEPT: { /* Blink for ~1 second, then go to next player */

			/*timeout code*/
			/*if currentPlayer.numPieces = 0*/
				/*remove current player*/
				/*setState(PLAYER_FINAL_MOVE);*/
			/*else*/
				/*nextPlayer()*/

			setState(PIECE_SELECT); break;
			} break;

		case PLAYER_FINAL_MOVE: { /* display 'player X is out' message, then go to next player */

			/*possible timeout, or wait for keypress*/
			/*if all players.numPieces = 0*/
				/*setState(GAME_OVER)*/
			/*else*/
				/*nextPlayer()*/
			setState(PIECE_SELECT); break;
			} break;

		case GAME_OVER: { /* display 'player X won' or possible ties */

			/*continue*/
			if (keys[VK_RETURN] == keyState::PRESSED) { setState(STATS); break; }
			} break;

		case STATS: { /* display stats for each player */

			/*back to menu*/
			if (keys['M'] == keyState::PRESSED) { setState(MENU); break; }
			/*quit*/
			if (keys['Q'] == keyState::PRESSED) { setState(QUIT); break; }
			} break;

		case QUIT: {

			if (keys[VK_ESCAPE] == keyState::PRESSED) { quitConsole(); break; }
			if (keys['B'] == keyState::PRESSED) { undoState(); break; }

			} break;
		}
	}

	virtual void draw() final {
		if (state == QUIT) {
			canvas.text(0, 0, L"Are you sure you want to quit?");
			canvas.text(0, 39, L"[ESC quit]      [B back]");
			return; // don't clear screen
		}
		canvas.clear();

		/* Draw the bpard */
		if (Is_Either(state, VIEW_BOARD, PIECE_SELECT, PIECE_MOVE ,
		              POSITION_ACCEPT, POSITION_REJECT ,
		              PLAYER_FINAL_MOVE, GAME_OVER)) {
			canvas.squareBorder({ 9, 9,31,31}, CHAR_INFO{L'█', 0x000F});
			canvas.drawImplicit({10,10,30,30}, CHAR_INFO{L'█', 0x0008} ,
				[&](signed x, signed y) { return (x&1) ^ (y&1); });
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
			canvas.text(0, 0, L"Arrow keys to select piece type & orientation");
			canvas.text(0, 39, L"[ENTER continue]");
		} break;

		case PIECE_MOVE: {
			canvas.text(0, 0, L"Move the piece around & choose permutation with A or D");
			canvas.text(0, 39, L"[ENTER continue]");

			canvasType::bounds pieceBounds = {cursorX, cursorY, cursorX + currentSize.n, cursorY + currentSize.m};
			canvas.drawImplicit(pieceBounds, CHAR_INFO{L'█', 0x0004} ,
				[&](signed x, signed y) { return selected.getShape()[y-cursorY, x-cursorX]; });
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

struct parseArguments {
	/*settings go here*/

	parseArguments(int argc, char* args[]) {}
};

int main(int argc, char* args[]) {
	auto settings = parseArguments{argc, args};
	MainApp app{L"Blokus Experiments", 
               {.width = 80, .height = 40, .fontW = 8, .fontH = 16}};

    app.start();
    while (!app.quit()) {
    	app.run();
    }

	return 0;
}
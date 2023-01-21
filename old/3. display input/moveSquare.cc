#include "console.hh"
#include "draw.hh"

const short WIDTH  = 60;
const short HEIGHT = 30;

float squareX, squareY;

class Moving_Square : public ConsoleDisplay {
	virtual void setup() {
		title = L"Alsin Moves a Square";

		squareX = 10;
		squareY = 10;
	}

	virtual void run() {
		/* RUN STUFF */
		if (keys['W'] || keys[VK_UP]   ) { squareY -= 10*Δtime; }
		if (keys['S'] || keys[VK_DOWN] ) { squareY += 10*Δtime; }
		if (keys['A'] || keys[VK_LEFT] ) { squareX -= 20*Δtime; }
		if (keys['D'] || keys[VK_RIGHT]) { squareX += 20*Δtime; }

		/* DRAW STUFF */
		Draw::clearScreen();

		int posX = (int)fmod(squareX , (WIDTH +10));
		int posY = (int)fmod(squareY , (HEIGHT+10));
		Draw::rect(posX, posY, posX+10, posY+5);
	}
};

int main(int argc, char* argv[]) {
	Moving_Square game;
	game.initConsole(WIDTH, HEIGHT);
	Draw::setConsole(&game);

	game.start();

	return 0;
}
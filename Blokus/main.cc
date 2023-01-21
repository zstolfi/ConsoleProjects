#include "console.hh"
#include "draw.hh"

#include "symmetry.hh"
#include "matrix.hh"
#include "pieces.hh"

/* -------------- */
/*  App Behavior  */
/* -------------- */

class MainApp : public ConsoleWindow {
public: /* Initialize Canvas */
	const unsigned width = getWidth()/2, height = getHeight();
	using canvasType = DrawSquare<std::vector<CHAR_INFO>, WCHAR, WORD>;

	canvasType canvas{
		getScreenBuffer(), getWidth(), getHeight() ,
		[](CHAR_INFO& pix, WCHAR chr) { pix.Char.UnicodeChar = chr; } ,
		[](CHAR_INFO& pix, WORD  col) { pix.Attributes       = col; }
	};

	/* Constructor */
	using ConsoleWindow::ConsoleWindow;

private: /* Game Variables */

	virtual void setup() final {
		canvas.blankPix = CHAR_INFO{L' ', 0x000F};
		canvas.clear();
		// noDraw = true;
	}

	virtual void update() final {
		if (keys[VK_ESCAPE] == keyState::PRESSED) { quitConsole(); }
	}

	virtual void draw() final {
		canvas.clear();
		for (unsigned x=0; x < width ; x++) {
		for (unsigned y=0; y < height; y++) {
			canvas.squareBorder({ 9, 9,31,31}, CHAR_INFO{L'█', 0x0008});
			canvas.drawImplicit({10,10,30,30}, CHAR_INFO{L'█', 0x0004} ,
				[&](unsigned x, unsigned y) { return (x&1) ^ (y&1); });
		} }
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
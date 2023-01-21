#include <complex>
#include <algorithm>
#include "console.hh"
#include "draw.hh"



/* -------------- */
/*   Define App   */
/* -------------- */

using complex = std::complex<double>;

class MainApp : public ConsoleWindow {
public: /* Initalize Draw Class */
	const unsigned width  = getWidth(), height = getHeight();
	using canvasType = Draw<std::vector<CHAR_INFO>, WCHAR, WORD>;
	canvasType screen{
		getScreenBuffer(), width, height ,
		[](CHAR_INFO& p, WCHAR chr) { p.Char.UnicodeChar = chr; } ,
		[](CHAR_INFO& p, WORD  col) { p.Attributes       = col; }
	};


	// Note: If command args need to go into MainApp, the constructor
	//       should take the settings of interest as arguments
	//       (already parsed) 
	MainApp(std::wstring title, initSettings s)
	: ConsoleWindow{title, s} {}

private: /* Game Variables */
	double W, H;
	double aspectRatioX, aspectRatioY;
	const std::array<wchar_t,2> checker = {L' ', L'█'};
	using enum keyState;

	virtual void setup() final {
		W = width / 2, H = height;
		aspectRatioX = std::max(1.0, (double)width/height);
		aspectRatioY = std::max(1.0, (double)height/width);

		// noDraw = true;
		screen.clear();
	}

	virtual void update() final {
		if (keys[VK_ESCAPE] == PRESSED) { quitConsole(); }
	}

	virtual void draw() final {
		// screen.clear();

		const unsigned frameWait = 1;
		if (frameCount % frameWait == 0) {
			std::size_t i = frameCount / frameWait;
			if (i+1 >= width*height) { noDraw = true; return; }
		// for (std::size_t i=0; i < width*height; i++) {
			unsigned xi = i % width / 2;
			unsigned yi = i / width;
			double x = (xi+0.5 - W/2) * 2.0 / std::min(W,H);
			double y = (yi+0.5 - H/2) * 2.0 / std::min(W,H);

			// begin per-pixel calculation
			const std::size_t maxIterations = 10000;

			complex z {0,0};
			complex c {x,y};

			bool tileColor = false;
			for (std::size_t i=0; std::abs(z) < 2.0 && i < maxIterations; i++) {
				z = z*z + c;
				if (i+1 == maxIterations) { tileColor = true; }
			}

			// bool tileColor = (xi%2 == 0) ^ (yi%2 == 0);
			// bool tileColor = x*x + y*y <= 1;
			screen.setChar (i, checker[tileColor]);
			screen.setColor(i, 0x000F);
		}
	}
};



/* -------------- */
/*      Main      */
/* -------------- */

struct parseArguments {
	complex location = {0,0};
	double zoom = 2.0;

	parseArguments(int argc, char* args[]) {
		if (argc > 1) { 
			std::string s{args[1]};
			if (s=="-h" || s=="-H" || s=="-help") { /*printHelp(); std::exit(0);*/ }
		}
		if (argc > 2) { location = {std::atof(args[1]), std::atof(args[2])}; }
		if (argc > 3) { zoom = std::atof(args[3]); }
	}
};

int main(int argc, char* args[]) {
	auto settings = parseArguments{argc, args};
	MainApp app{L"Mandelbrot Experiments" ,
	           {.width = 120, .height = 40, .fontW = 8, .fontH = 16}};

	app.start();
	while (!app.quit()) {
		app.run();
	}

	return 0;
}
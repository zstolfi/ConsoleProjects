#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

const int WIDTH  = 80;
const int HEIGHT = 40;

HANDLE consoleWrite;
HANDLE consoleRead;
SMALL_RECT windowRect;
COORD bufferCoord;

void setup() {
	consoleWrite = GetStdHandle(STD_OUTPUT_HANDLE);
	consoleRead  = GetStdHandle(STD_INPUT_HANDLE);

	SetConsoleTitleW(L"Alsin's Epic Program");

	windowRect = {0, 0, (short)WIDTH-1, (short)HEIGHT-1};
	SetConsoleWindowInfo(consoleWrite, true, &windowRect);

	bufferCoord = {(short)WIDTH, (short)HEIGHT};
	SetConsoleScreenBufferSize(consoleWrite, bufferCoord);
}

void draw() {

}

int main(int argc, char* argv[]) {
	
	setup();

	while (true) {
		draw();
	}

	return 0;
}
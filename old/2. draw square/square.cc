#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <algorithm>
#include <chrono>

const short WIDTH  = 80;
const short HEIGHT = 40;

std::wstring title;
HANDLE consoleWrite;
HANDLE consoleRead;
SMALL_RECT windowRect;
COORD bufferCoord;

CHAR_INFO* charBuffer;

int frameCount;
auto time0 = std::chrono::system_clock::now();
auto time1 = std::chrono::system_clock::now();
float Δtime;

void setup() {
	title = L"Alsin's Drawing Squares";

	consoleWrite = GetStdHandle(STD_OUTPUT_HANDLE);
	consoleRead  = GetStdHandle(STD_INPUT_HANDLE);

	SetConsoleTitleW(title.c_str());

	CONSOLE_CURSOR_INFO cursor = {1, false};
	SetConsoleCursorInfo(consoleWrite, &cursor);

	windowRect = {0, 0, WIDTH-1, HEIGHT-1};
	SetConsoleWindowInfo(consoleWrite, true, &windowRect);
	bufferCoord = {WIDTH, HEIGHT};
	SetConsoleScreenBufferSize(consoleWrite, bufferCoord);
	SetConsoleActiveScreenBuffer(consoleWrite);

	charBuffer = new CHAR_INFO[WIDTH * HEIGHT];
	// memset(charBuffer, 0, sizeof(CHAR_INFO) * WIDTH * HEIGHT);


	frameCount = 0;
}

double shape(double x, double y) {
	double x1 = x-40, y1 = 2*y-40;
	return abs(std::max(abs(x1), abs(y1)) - 30) - 2;

	// int frame = frameCount % (WIDTH * HEIGHT);
	// return abs(y * WIDTH + x - frame) - 5;

	// return 0;
}

void draw() {
	for (int y=0; y < HEIGHT; y++) {
	for (int x=0; x < WIDTH;  x++) {
		int i = y * WIDTH + x;
		charBuffer[i].Char.UnicodeChar = L'*';

		WORD color = (shape(x,y) <= 0) ? rand()%16 : 0x0000;
		charBuffer[i].Attributes = color;
	} }

	wchar_t titleBuffer[256];
	swprintf_s(titleBuffer, 256, L"%s - fps: %4.1f", title.c_str(), 1.0/Δtime);

	WriteConsoleOutputW(consoleWrite, charBuffer, {WIDTH,HEIGHT}, {0,0}, &windowRect);
	SetConsoleTitleW(titleBuffer);
}



void updateTime() {
	time1 = std::chrono::system_clock::now();
	Δtime = std::chrono::duration<float>(time1 - time0).count();
	time0 = time1;

	frameCount++;
}

int main(int argc, char* argv[]) {
	
	setup();

	while (true) {
		draw();

		updateTime();
	}

	return 0;
}
#pragma once

// #include <utility>

const wchar_t* boardTemplate = L""
"╔══════╤══════╤══════╤══════╗\n"
"║      │      │      │      ║\n"
"║00~~~~│01~~~~│02~~~~│03~~~~║\n"
"║      │      │      │      ║\n"
"╟──────┼──────┼──────┼──────╢\n"
"║      │      │      │      ║\n"
"║10~~~~│11~~~~│12~~~~│13~~~~║\n"
"║      │      │      │      ║\n"
"╟──────┼──────┼──────┼──────╢\n"
"║      │      │      │      ║\n"
"║20~~~~│21~~~~│22~~~~│23~~~~║\n"
"║      │      │      │      ║\n"
"╟──────┼──────┼──────┼──────╢\n"
"║      │      │      │      ║\n"
"║30~~~~│31~~~~│32~~~~│33~~~~║\n"
"║      │      │      │      ║\n"
"╚══════╧══════╧══════╧══════╝";

enum Dir {
	UP, DOWN, LEFT, RIGHT
};

typedef std::pair<int, int> Pos;
Pos directionTable[][4][4] = {
	{	// UP
		{{0,0}, {1,0}, {2,0}, {3,0}} ,
		{{0,1}, {1,1}, {2,1}, {3,1}} ,
		{{0,2}, {1,2}, {2,2}, {3,2}} ,
		{{0,3}, {1,3}, {2,3}, {3,3}} ,
	} ,
	
	{	// DOWN
		{{3,3}, {2,3}, {1,3}, {0,3}} ,
		{{3,2}, {2,2}, {1,2}, {0,2}} ,
		{{3,1}, {2,1}, {1,1}, {0,1}} ,
		{{3,0}, {2,0}, {1,0}, {0,0}} ,
	} ,

	{	// LEFT
		{{0,0}, {0,1}, {0,2}, {0,3}} ,
		{{1,0}, {1,1}, {1,2}, {1,3}} ,
		{{2,0}, {2,1}, {2,2}, {2,3}} ,
		{{3,0}, {3,1}, {3,2}, {3,3}} ,
	} ,
	
	{	// RIGHT
		{{0,3}, {0,2}, {0,1}, {0,0}} ,
		{{1,3}, {1,2}, {1,1}, {1,0}} ,
		{{2,3}, {2,2}, {2,1}, {2,0}} ,
		{{3,3}, {3,2}, {3,1}, {3,0}} ,
	}
};
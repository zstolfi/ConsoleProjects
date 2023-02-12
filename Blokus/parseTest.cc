#include "board history.hh"
#include <fstream>

int main(int argc, char* args[]) {
	if (argc <= 1) { std::cerr << "Error!: No game history provided\n"; return 1; }
	std::ifstream file{args[1]};
	if (!file) { std::cerr << "Error!: Failed to open \"" << args[1] << "\"\n"; return 2; }
	std::stringstream historyStr;
	historyStr << file.rdbuf();

	std::cout << "\nstart\n\n";

	BoardHistory test;
	if (auto value = readGame(historyStr)) {
		test = *value;
	} else {
		std::cout << "Parse Error! (But no termination)";
		return 1;
	}

	std::cout << "In this game, there are " << test.numPlayers << " players.\n";
	std::cout << "The corner numbers are:";
	for (unsigned corner : test.playerOrder) { std::cout << " " << corner; }
	std::cout << "\n";
	std::cout << "The game has a total of " << test.movesList.size() << " moves.\n";
	std::cout << "The game is " << (test.getValidity() == BoardHistory::VALID ? "valid" : "not valid") << "\n";

	std::cout << "\nend\n";
	return 0;
}
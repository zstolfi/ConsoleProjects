#include <fstream>
#include "board history.hh"

int main(int argc, char* args[]) {
	if (argc <= 1) { std::cerr << "Error!: No game history provided\n"; return 1; }
	std::ifstream file{args[1]};
	if (!file) { std::cerr << "Error!: Failed to open \"" << args[1] << "\"\n"; return 2; }
	std::stringstream historyStr;
	historyStr << file.rdbuf();

	std::cout << "start\n";

	/*BoardHistory test = */ParseHistory(historyStr);

	std::cout << "end\n";
	return 0;
}
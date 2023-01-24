#pragma once
#include "common.hh"

class BoardHistory {

};



#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <pair>

enum class fileFormat { CANONICAL };

template <fileFormat format = fileFormat::CANONICAL>
BoardHistory ParseHistory(std::istringstream str) {
	/* preprocess */
	// 1st pass, store comment locations, (parenthesis)
	std::map<std::streampos, std::streampos> commentRanges;

	/* tokenize */
	// (I'm actually not too familair with interpreter/compiler terms,
	// so tokenize might not be the right name...)
	enum dataType {
		PLAYER_COUNT ,
		PLYER ORDER ,
		COLOR_DATA ,
		PIECE_DATA
	};
	std::vector<std::pair<dataType,std::string>> tokens {};

	enum {

	} state;

	/* parse pre-game options */
	BoardHistory board{numPlayers, playerOrder};
	/**/
}
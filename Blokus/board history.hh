#pragma once
#include "common.hh"
#include <vector>
#include <variant>

namespace /*private*/ {
	struct PieceID {
		unsigned num, rot;
	};

	struct NoMove {};

	struct PlayerMove {
		PieceID id;
		unsigned x, y;
	};
}

struct BoardHistory {
	unsigned numPlayers;
	std::vector<unsigned> playerOrder;
	// player color is not needed by the computer
	std::vector<std::variant<PlayerMove,NoMove>>;
};



#include <iostream>
#include <sstream>
#include <string_view>
#include <map>
#include <utility> // std::pair
#include <optional>

namespace /*private*/ {
	// using streamPos = std::streampos;
	// using streamOff = std::streamoff;
	using ssRange     = std::pair<std::streampos, std::streampos>;
	using ssRangeList = std::map <std::streampos, std::streampos>;

	constexpr isWhitespace(char c) { return Is_Either(c,' ',',','\r','\n','\t'); }

	// if we fail, move the curor back to the start
	#define returnFail str.seekg(result.first ); return std::nullopt;
	// otherwise, set the cursor back 1 position
	#define returnPass str.seekg(result.second); return result;

	// assets/canonical parse diagram.png
	using parseFunc = std::optional<ssRange>(std::stringstream&);
	parseFunc parse_CANNONICAL_FORMAT, parse_INT;


	std::optional<ssRange> parseID(std::stringstream& str) {
		ssRange result = {str.tellg(), str.tellg()};
		enum { X, POINT, Y } state = X;
		for (char c; str.get(c); result.second += std::streamoff{1}) {
			if (state == X) {
				if(!parseINT(str)) { returnFail; }
				state = POINT;
			} else if (state == POINT) {
				if (c != '.') { returnFail; }
				state = Y;
			} else if (state == Y) {
				if (!parseINT(str)) { returnFail; }
				break;
			}
		}
		returnPass;
	}

	std::optional<ssRange> parseINT(std::stringstream& str) {
		ssRange result = {str.tellg(), str.tellg()};
		enum { FIRST, REST } state = FIRST;
		for (char c; str.get(c); result.second += std::streamoff{1}) {
			if (state == FIRST) {
				if (!('0' <= c&&c <= '9')) { returnFail; }
				state = REST;
			} else if (state == REST) {
				if (!('0' <= c&&c <= '9')) { break; }
			}
		}
		returnPass;
	}

	std::optional<ssRange> parseWhitespace(std::stringstream& str) {
		ssRange result = {str.tellg(), str.tellg()};
		enum { FIRST, REST } state = FIRST;
		for (char c; str.get(c); result.second += std::streamoff{1}) {
			if (state == FIRST) {
				if (!isWhitespace(c)) { returnFail; }
				state = REST;
			} else if (state == REST) {
				if (!isWhitespace(c)) { break; }
			}
		}
		returnPass;
	}

	#undef returnFail
	#undef returnPass
}

enum class fileFormat { CANONICAL };

template <fileFormat format = fileFormat::CANONICAL>
BoardHistory ParseHistory(std::stringstream& strRaw) {
	std::cout << "Parsing History...\n";
	/* preprocess */
	// Create 'str' variable wich is just strRaw without comments or leading/trailing whitespaces
	ssRangeList commentRanges = {};
	ssRangeList whitespaceRanges = {};

	bool insideParen = false;
	for (char c; strRaw.get(c); ) {
		std::streampos i0 = strRaw.tellg() - std::streamoff{1};
		std::streampos i1 = strRaw.tellg();

		bool prevInsideParen = insideParen;
		if (c == '(') { insideParen = true;  commentRanges[i0]; } // TODO: check for mismatching parenthesis
		if (c == ')') { insideParen = false; commentRanges.rbegin()->second = i1; }

		if (isWhitespace(c) || insideParen || prevInsideParen) {
			auto rbegin = whitespaceRanges.rbegin();
			auto rend   = whitespaceRanges.rend();
			if (rbegin != rend && rbegin->second == i0)
				rbegin->second += std::streamoff{1};
			else
				whitespaceRanges[i0] = i1;
		}
	}

	std::cout << "There are " << commentRanges.size() << " comments.\n";
	for (const auto& [start, end] : commentRanges) {
		std::cout << "\t" << start << " - " << end << "\n";
	}

	std::cout << "There are " << whitespaceRanges.size() << " groups of whitespace chars.\n";
	auto wsFirst = whitespaceRanges.begin();
	auto wsLast  = whitespaceRanges.rbegin();
	std::cout << "\tThe first is: " << wsFirst->first << " - " << wsFirst->second << "\n";
	std::cout << "\tThe last is: "  << wsLast ->first << " - " << wsLast ->second << "\n";

	strRaw.clear(); strRaw.seekg(0, std::ios_base::end);
	ssRange strRange = {0, strRaw.tellg()};

	if (wsFirst->first  == strRange.first ) { commentRanges[wsFirst->first] = wsFirst->second; }
	if (wsLast ->second == strRange.second) { commentRanges[wsLast ->first] = wsLast ->second; }

	std::stringstream str{};

	strRaw.clear(); strRaw.seekg(0);
	for (char c; strRaw.get(c); ) {
		std::streampos i = strRaw.tellg() - std::streampos{1};
		if (commentRanges.contains(i)) { strRaw.seekg(commentRanges.at(i)); continue; }
		str.put(c);
	}

	std::cout << "\"" << str.str() << "\"\n";

	// /* tokenize */
	// // (I'm actually not too familair with interpreter/compiler terms,
	// // so tokenize might not be the right name...)
	// enum dataType {
	// 	PLAYER_COUNT ,
	// 	PLYER ORDER ,
	// 	COLOR_DATA ,
	// 	PIECE_DATA
	// };
	// std::vector<std::pair<dataType,std::string_view>> tokens {};

	// enum {

	// } state;

	// /* parse pre-game options */

	BoardHistory board{};
	return board;
}
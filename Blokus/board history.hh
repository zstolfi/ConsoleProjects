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

	using numPlayers_t  = unsigned;
	using playerOrder_t = std::vector<unsigned>;
	using move_t        = std::variant<PlayerMove, NoMove>;
	using movesList_t   = std::vector<move_t>;
}

struct BoardHistory {
	numPlayers_t numPlayers;
	playerOrder_t playerOrder;
	// player color is not needed by the computer
	movesList_t movesList;
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

	// forward declare all parse functions, so we can write them
	// in any order we want
	template <typename T> using parseFunc    = std::optional<T>/**/(std::stringstream&);
	template <typename T> using parseFuncEOF = std::optional<T>/**/(std::stringstream&,auto&&);
	/*                 */ using parseFuncNoReturn =        bool/**/(std::stringstream&);

	parseFuncEOF<BoardHistory>  parse_CANONICAL_FORMAT;
	parseFunc<playerOrder_t>    parse_PLAYER_ORDER;
	parseFuncNoReturn           parse_COLOR_DATA;
	parseFunc<PlayerMove>       parse_PIECE_POS;
	parseFunc<PieceID>          parse_ID;
	parseFunc<unsigned>         parse_INT;
	parseFuncNoReturn           parse_whitespace;

	// if we fail, move the curor back to the start
	#define returnFail str.seekg(range.first); return {}; // nullopt, or false
	// otherwise, set the cursor back 1 position
	#define returnPass str.seekg(range.second); return result;

	// nextEOF is a predicate functor which says if the current char is the last char
	std::optional<BoardHistory> parse_CANONICAL_FORMAT(std::stringstream& str, auto&& lastChar) {
		BoardHistory result = {};
		ssRange range = {str.tellg(), str.tellg()};
		enum { NUM, ws1, ORDER, ws2, COLOR, ws3, MOVE, ws4 } state = NUM;
		for (char c; str.get(c); range.second += std::streamoff{1}) {
			if (state == NUM) {
				auto numPlayers = parse_INT(str);
				if (!numPlayers) { returnFail; }
				result.numPlayers = numPlayers;
				state = ws1;
			} else if (state == ws1) {
				if (!parse_whitespace(str)) { returnFail; }
				state = ORDER;
			} else if (state == ORDER) {
				auto playerOrder = parse_PLAYER_ORDER(str);
				if (!playerOrder) { returnFail; }
				result.playerOrder = playerOrder;
				if (lastChar()) { break; }
				state = ws2;
			} else if (state == ws2) {
				if (!parse_whitespace(str)) { returnFail; }
				if (str.peek() == 'c') { state = COLOR; }
				else { state = MOVE; }
			} else if (state == COLOR) {
				if (!parse_COLOR_DATA(str)) { returnFail; }
				if (lastChar()) { break; }
				state = ws3;
			} else if (state == ws3) {
				if (!parse_whitespace(str)) { returnFail; }
				state = MOVE;
			} else if (state == MOVE) {
				if (c == 'x') {
					result.movesList.push_back(NoMove{});
				} else {
					auto move = parse_PIECE_POS(str);
					if (!move) { returnFail; }
					result.movesList.push_back(move);
				}
				if (lastChar()) { break; }
				state = ws4;
			} else if (state == ws4) {
				if (!parse_whitespace(str)) { returnFail; }
				state = MOVE;
			}
		}
		returnPass;
	}

	std::optional<playerOrder_t> parse_PLAYER_ORDER(std::stringstream& str) {
		playerOrder_t result = {};
		ssRange range = {str.tellg(), str.tellg()};
		enum { FIRST, REST } state = FIRST;
		for (char c; str.get(c); range.second += std::streamoff{1}) {
			if (state == FIRST) {
				if (!('0' <= c&&c <= '9')) { returnFail; }
				result.push_back(c-'0');
				state = REST;
			} else if (state == REST) {
				if (!('0' <= c&&c <= '9')) { break; }
				result.push_back(c-'0');
			}
		}
		returnPass;
	}

	bool parse_COLOR_DATA(std::stringstream& str) {
		bool result = true;
		ssRange range = {str.tellg(), str.tellg()};
		enum { TAG, FIRST, REST } state = FIRST;
		for (char c; str.get(c); range.second += std::streamoff{1}) {
			if (state == TAG) {
				if (!(c == 'c' && str.get() == ':')) { returnFail; }
				state = FIRST;
			} else if (state == FIRST) {
				if (!(('a' <= c&&c <= 'z') || ('A' <= c&&c <= 'Z'))) { returnFail; }
				state = REST;
			} else if (state == REST) {
				if (!(('a' <= c&&c <= 'z') || ('A' <= c&&c <= 'Z'))) { break; }
			}
		}
		returnPass;
	}

	std::optional<PlayerMove> parse_PIECE_POS(std::stringstream& str) {
		PlayerMove result = {};
		ssRange range = {str.tellg(), str.tellg()};
		enum { ID, ws1, X, ws2, Y} state = ID;
		for (char c; str.get(c); range.second += std::streamoff{1}) {
			if (state == ID) {
				auto id = parse_ID(str);
				if (!id) { returnFail; }
				result.id = id;
				state = ws1;
			} else if (state == ws1) {
				if (!parse_whitespace(str)) { returnFail; }
				state = X;
			} else if (state == X) {
				auto x = parse_INT(str);
				if (!x) { returnFail; }
				result.x = x;
				state = ws2;
			} else if (state == ws2) {
				if (!parse_whitespace(str)) { returnFail; }
				state = Y;
			} else if (state == Y) {
				auto y = parse_INT(str);
				if (!y) { returnFail; }
				result.y = y;
				break;
			}
		}
		returnPass;
	}

	std::optional<PieceID> parse_ID(std::stringstream& str) {
		PieceID result = {0,0};
		ssRange range = {str.tellg(), str.tellg()};
		enum { NUM, POINT, ROT } state = NUM;
		for (char c; str.get(c); range.second += std::streamoff{1}) {
			if (state == NUM) {
				auto num = parse_INT(str);
				if(!num) { returnFail; }
				result.num = num;
				state = POINT;
			} else if (state == POINT) {
				if (c != '.') { returnFail; }
				state = ROT;
			} else if (state == ROT) {
				auto rot = parse_INT(str);
				if (!rot) { returnFail; }
				result.rot = rot;
				break;
			}
		}
		returnPass;
	}

	std::optional<unsigned> parse_INT(std::stringstream& str) {
		unsigned result = 0;
		ssRange range = {str.tellg(), str.tellg()};
		enum { FIRST, REST } state = FIRST;
		for (char c; str.get(c); range.second += std::streamoff{1}) {
			if (state == FIRST) {
				if (!('0' <= c&&c <= '9')) { returnFail; }
				result = c-'0';
				state = REST;
			} else if (state == REST) {
				if (!('0' <= c&&c <= '9')) { break; }
				result = 10*result + (c-'0')
			}
		}
		returnPass;
	}

	bool parse_whitespace(std::stringstream& str) {
		bool result = true;
		ssRange range = {str.tellg(), str.tellg()};
		enum { FIRST, REST } state = FIRST;
		for (char c; str.get(c); range.second += std::streamoff{1}) {
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
// TODO: change strRaw back into a istringstream
BoardHistory readGame(std::stringstream& strRaw) {
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

	return parse_CANONICAL_FORMAT(str, [&] { return str.tellg() == strRage.second; });
}

void writeGame(BoardHistory&, std::ostringstream& str);
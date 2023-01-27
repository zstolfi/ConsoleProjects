#pragma once
#include "common.hh"
#include <vector>
#include <variant>

struct PieceID { unsigned num, rot; };
struct NoMove {};
struct PlayerMove { PieceID id; unsigned x, y; };

using numPlayers_t  = unsigned;
using playerOrder_t = std::vector<unsigned>;
using move_t        = std::variant<PlayerMove, NoMove>;
using movesList_t   = std::vector<move_t>;

struct BoardHistory {
	numPlayers_t numPlayers;
	playerOrder_t playerOrder;
	// player color is not needed by the computer
	movesList_t movesList;
};



#include <iostream>
#include <sstream>
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
	template <typename T> using parseFunc = std::optional<T>/**/(std::stringstream&);
	/*                 */ using parseFuncNoReturn =     bool/**/(std::stringstream&);

	parseFunc<playerOrder_t>    parse_PLAYER_ORDER;
	parseFuncNoReturn           parse_COLOR_DATA;
	parseFunc<PlayerMove>       parse_PIECE_POS;
	parseFunc<PieceID>          parse_ID;
	parseFunc<unsigned>         parse_INT;
	parseFuncNoReturn           parse_whitespace;

	// if we fail, move the curor back to the start
	#define returnFail \
		std::cout << __LINE__ << "\tparsing failed at streampos = " << str.tellg() \
		          << " and state = " << state << "\n"; \
		str.seekg(range.first); return {}; // nullopt, or false
	// otherwise, set the cursor back 1 position
	#define returnPass \
		/*std::cout << __LINE__ << "\t" << range.first << " - " << range.second << "\n";*/ \
		return result;

	#define nextChar() \
		str.seekg(std::streamoff{1}, std::ios_base::cur); \
		range.second = str.tellg(); \
		c = str.peek();

	#define Terminal(COND, SUCCEED, FAIL) \
		if (!(COND)) { FAIL } \
		SUCCEED \
		nextChar();

	#define TerminalStr(STR, SUCCEED, FAIL) \
		for (char d : STR) { \
			if (d == '\0') { continue; } \
			if (c != d) { FAIL } \
			nextChar(); \
		} SUCCEED

	void Assign   (auto&& a, auto&& b) { a = b; }
	void Push_Back(auto&& a, auto&& b) { a.push_back(b); }

	#define NonTerminal(TYPE, F, VAR, FAIL) \
		auto value = parse_##TYPE(str); \
		if (!value) { FAIL } \
		F(VAR, *value); \
		c = str.peek();

	#define NonTerminalNull(TYPE, FAIL) \
		if (!parse_##TYPE(str)) { FAIL } \
		c = str.peek();

	// lastChar is a predicate functor
	std::optional<BoardHistory> parse_CANONICAL_FORMAT(std::stringstream& str, auto&& lastChar) {
		BoardHistory result = {};
		ssRange range = {str.tellg(), str.tellg()};
		enum { NUM, ws1, ORDER, ws2, COLOR, ws3, MOVE, ws4 } state = NUM;
		while (str) {
			[[maybe_unused]] char c = str.peek();
			if (state == NUM) {
				NonTerminal(INT, Assign, result.numPlayers, returnFail; );
				state = ws1;
			} else if (state == ws1) {
				NonTerminalNull(whitespace, returnFail; );
				state = ORDER;
			} else if (state == ORDER) {
				NonTerminal(PLAYER_ORDER, Assign, result.playerOrder, returnFail; );
				if (lastChar()) { break; }
				state = ws2;
			} else if (state == ws2) {
				NonTerminalNull(whitespace, returnFail; );
				state = (c == 'c') ? COLOR : MOVE;
			} else if (state == COLOR) {
				NonTerminalNull(COLOR_DATA, returnFail; );
				if (lastChar()) { break; }
				state = ws3;
			} else if (state == ws3) {
				NonTerminalNull(whitespace, returnFail; );
				state = MOVE;
			} else if (state == MOVE) {
				if (c == 'x') {
					result.movesList.push_back(NoMove{});
					nextChar();
				} else {
					NonTerminal(PIECE_POS, Push_Back, result.movesList, returnFail; );
				}
				if (lastChar()) { break; }
				state = ws4;
			} else if (state == ws4) {
				NonTerminalNull(whitespace, returnFail; );
				state = MOVE;
			}
		}
		returnPass;
	}

	std::optional<playerOrder_t> parse_PLAYER_ORDER(std::stringstream& str) {
		playerOrder_t result = {};
		ssRange range = {str.tellg(), str.tellg()};
		enum { FIRST, REST } state = FIRST;
		while (str) {
			[[maybe_unused]] char c = str.peek();
			if (state == FIRST) {
				Terminal('0' <= c&&c <= '9', result.push_back(c-'0'); , returnFail; );
				state = REST;
			} else if (state == REST) {
				Terminal('0' <= c&&c <= '9', result.push_back(c-'0'); , break; )
			}
		}
		returnPass;
	}

	bool parse_COLOR_DATA(std::stringstream& str) {
		bool result = true;
		ssRange range = {str.tellg(), str.tellg()};
		enum { TAG, FIRST, REST } state = TAG;
		while (str) {
			[[maybe_unused]] char c = str.peek();
			if (state == TAG) {
				TerminalStr("c:", /**/; , returnFail; );
				state = FIRST;
			} else if (state == FIRST) {
				Terminal(('a' <= c&&c <= 'z') || ('A' <= c&&c <= 'Z'), /**/; , returnFail; );
				state = REST;
			} else if (state == REST) {
				Terminal(('a' <= c&&c <= 'z') || ('A' <= c&&c <= 'Z'), /**/; , break; );
			}
		}
		returnPass;
	}

	std::optional<PlayerMove> parse_PIECE_POS(std::stringstream& str) {
		PlayerMove result = {};
		ssRange range = {str.tellg(), str.tellg()};
		enum { ID, ws1, X, ws2, Y} state = ID;
		while (str) {
			[[maybe_unused]] char c = str.peek();
			if (state == ID) {
				NonTerminal(ID, Assign, result.id, returnFail; );
				state = ws1;
			} else if (state == ws1) {
				NonTerminalNull(whitespace, returnFail; );
				state = X;
			} else if (state == X) {
				NonTerminal(INT, Assign, result.x, returnFail; );
				state = ws2;
			} else if (state == ws2) {
				NonTerminalNull(whitespace, returnFail; );
				state = Y;
			} else if (state == Y) {
				NonTerminal(INT, Assign, result.y, returnFail; );
				break;
			}
		}
		returnPass;
	}

	std::optional<PieceID> parse_ID(std::stringstream& str) {
		PieceID result = {0,0};
		ssRange range = {str.tellg(), str.tellg()};
		enum { NUM, POINT, ROT } state = NUM;
		while (str) {
			[[maybe_unused]] char c = str.peek();
			if (state == NUM) {
				NonTerminal(INT, Assign, result.num, returnFail; );
				state = POINT;
			} else if (state == POINT) {
				Terminal(c == '.', /**/; , returnFail; );
				state = ROT;
			} else if (state == ROT) {
				NonTerminal(INT, Assign, result.rot, returnFail; );
				break;
			}
		}
		returnPass;
	}

	std::optional<unsigned> parse_INT(std::stringstream& str) {
		unsigned result = 0;
		ssRange range = {str.tellg(), str.tellg()};
		enum { FIRST, REST } state = FIRST;
		while (str) {
			[[maybe_unused]] char c = str.peek();
			if (state == FIRST) {
				Terminal('0' <= c&&c <= '9', result = c-'0'; , returnFail; );
				state = REST;
			} else if (state == REST) {
				Terminal('0' <= c&&c <= '9', result = 10*result + (c-'0'); , break; );
			}
		}
		returnPass;
	}

	bool parse_whitespace(std::stringstream& str) {
		bool result = true;
		ssRange range = {str.tellg(), str.tellg()};
		enum { FIRST, REST } state = FIRST;
		while (str) {
			[[maybe_unused]] char c = str.peek();
			if (state == FIRST) {
				Terminal(isWhitespace(c), /**/; , returnFail; );
				state = REST;
			} else if (state == REST) {
				Terminal(isWhitespace(c), /**/; , break; );
			}
		}
		returnPass;
	}

	#undef returnFail
	#undef returnPass
	#undef nextChar
	#undef Terminal
	#undef TerminalStr
	#undef NonTerminal
	#undef NonTerminalNull
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

	return parse_CANONICAL_FORMAT(str, [&] { return str.tellg() + std::streamoff{1} == strRange.second; })
	      .value_or(BoardHistory{});
}

void writeGame(BoardHistory&, std::ostringstream& str);
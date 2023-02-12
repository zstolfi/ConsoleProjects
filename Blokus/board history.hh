#pragma once
#include "common.hh"
#include "pieces.hh"
#include <vector>
#include <variant>

struct BoardSize { unsigned x, y; };
struct PieceID { unsigned num, rot; };
struct PlayerMove { PieceID id; unsigned x, y; };
struct NoMove {};

using boardSize_t   = BoardSize;
using numPlayers_t  = unsigned;
using playerOrder_t = std::vector<unsigned>;
using move_t        = std::variant<PlayerMove, NoMove>;
using movesList_t   = std::vector<move_t>;

struct BoardHistory {
	// TODO: make boardSize part of the grammar
	const boardSize_t boardSize = {20, 20};
	numPlayers_t numPlayers;
	playerOrder_t playerOrder;
	// player color is not needed by the computer
	movesList_t movesList;

	enum boardState { VALID ,
		NO_PLAYERS, TOO_MANY_PLAYERS, PLAYER_ORDER_MISMATCH ,
		EARLY_MOVE_SKIP, MOVE_AFTER_FINAL ,
		PIECE_OUT_OF_BOUNDS
	};

	boardState getValidity() {
		/* game checking logic */
		if (numPlayers <= 0) { return NO_PLAYERS; }
		if (numPlayers >  4) { return TOO_MANY_PLAYERS; }
		if (playerOrder.size() != numPlayers) { return PLAYER_ORDER_MISMATCH; }
		
		/* spatial checks, unoptimized */
		for (const auto& move : movesList) {
			bool movesAvailable = false;
			/* logic, assume true for now */
			movesAvailable = true;
			// /**/ if (std::holds_alternative<NoMove>(move)     &&  movesAvailable) { return EARLY_MOVE_SKIP; }
			// else if (std::holds_alternative<PlayerMove>(move) && !movesAvailable) { return MOVE_AFTER_FINAL; }
			if (std::holds_alternative<PlayerMove>(move)) {
				/* bounds check */
				const auto& curPiece = Pieces[move.id.num].getOption[move.id.rot];
				const auto& curShape = curPiece.getShape();
				const auto& size = curShape.size();
				if (move.x < 0 || move.x + size.n >= boardSize.x
				 || move.y < 0 || move.y + size.m >= boardSize.y) {
					return PIECE_OUT_OF_BOUNDS;
				}
			}
		}

		return VALID;
	}
};



#include <iostream>
#include <sstream>
#include <map>
#include <utility> // std::pair
#include <optional>
#include <functional>

namespace /*private*/ {
	using ssRange     = std::pair<std::streampos, std::streampos>;
	using ssRangeList = std::map <std::streampos, std::streampos>;

	// forward declare all parse functions, so we can write them
	// in any order we want
	template <typename T> using parseFunc    = std::optional<T>/**/(std::stringstream&);
	/*                 */ using parseFuncNoReturn =        bool/**/(std::stringstream&);

	parseFunc<BoardHistory>     parse_CANONICAL_FORMAT;
	parseFunc<playerOrder_t>    parse_PLAYER_ORDER;
	parseFuncNoReturn           parse_COLOR_DATA;
	parseFunc<PlayerMove>       parse_PIECE_POS;
	parseFunc<PieceID>          parse_ID;
	parseFunc<unsigned>         parse_INT;
	parseFuncNoReturn           parse_whitespace;

	unsigned depth = 0;

	#define Loop_Start() \
		depth++; \
		[[maybe_unused]] char c = str.peek();

	// if we fail, move the curor back to the start
	#define return_Fail \
		std::cout << __LINE__ << "\tparsing failed at streampos = " << str.tellg() \
		          << " and state = " << state << "\n"; \
		str.seekg(range.first); return {}; // nullopt, or false
	// otherwise, set the cursor back 1 position
	#define return_Pass \
		/*std::cout << __LINE__ << "\t";*/ \
		/*for (std::size_t i=0; i < depth; i++) { std::cout << "\t"; }*/ \
		/*std::cout << range.first << " - " << range.second << "\n";*/ \
		depth--; \
		return result;

	#define Logic_Error() \
		std::cout << "Logic Error:\n"; \
		return_Fail;

	#define Logic_Check() \
		if (auto err = result.getValidity(); err != BoardHistory::boardState::VALID) { \
			Logic_Error(); \
		}

	#define Next_Char(OFF) \
		str.seekg(std::streamoff{OFF}, std::ios_base::cur); \
		range.second = str.tellg(); \
		c = str.peek();

	#define Terminal(COND, SUCCEED, FAIL) \
		if (COND) { SUCCEED } \
		else { FAIL } \
		Next_Char(1);

	#define TerminalStr(STR, SUCCEED, FAIL) \
		for (std::size_t i=0; i < sizeof STR -1; i++) { \
			if (c != STR[i]) { FAIL } \
			Next_Char(1); \
		} SUCCEED

	template <typename T> struct add_optional { using type = std::optional<T>; };
	template <>     struct add_optional<bool> { using type = bool; };
	template <typename T> using add_optional_t = typename add_optional<T>::type;

	template <typename T> T    Remove_Optional(std::optional<T> value) { return *value; }
	                      bool Remove_Optional(bool             value) { return  value; }

	#define NonTerminal(TYPE, SUCCEED, FAIL) \
		if (auto optValue = parse_##TYPE(str)) { \
			[[maybe_unused]] auto value = Remove_Optional(optValue); \
			SUCCEED \
		} else { FAIL } \
		Next_Char(0);

	// TODO: remake non-terminals into a class system, instead of
	//       using macros
	#define NT(NAME, TYPE, INITAL_VAL, FIRST_ENUM, ...) \
		add_optional_t<TYPE> parse_##NAME(std::stringstream& str) { \
			ssRange range = {str.tellg(), str.tellg()}; \
			enum { FIRST_ENUM, __VA_ARGS__ } state = FIRST_ENUM; \
			Loop_Start(); \
			TYPE result = INITAL_VAL; \
			while (str) { \
				switch (state) {

	#define NT_End() \
		} } return_Fail; }



	constexpr isWhitespace(char c) { return Is_Either(c,' ',',','\r','\n','\t'); }

	std::optional<BoardHistory> parse_CANONICAL_FORMAT(std::stringstream& str) {
		ssRange range = {str.tellg(), str.tellg()};
		enum { SETTINGS, COLOR, MOVE } state = SETTINGS;
		Loop_Start();

		BoardHistory result {};

		while (str) {
			switch (state) {
			case SETTINGS:
				NonTerminal(INT, result.numPlayers = value; , return_Fail; );
				Logic_Check();
				NonTerminal(whitespace, /**/; , return_Fail; );
				NonTerminal(PLAYER_ORDER, result.playerOrder = value; , return_Fail; );
				Logic_Check();
				if (str.eof()) { return_Pass; }
				NonTerminal(whitespace, /**/; , return_Fail; );
				state = (c == 'c') ? COLOR : MOVE;
				break;
			case COLOR:
				NonTerminal(COLOR_DATA, /**/; , return_Fail; );
				if (str.eof()) { return_Pass; }
				NonTerminal(whitespace, /**/; , return_Fail; );
				state = MOVE;
				break;
			case MOVE:
				if (c == 'x') {
					Terminal(true, result.movesList.push_back(NoMove{}); , /**/; );
					Logic_Check();
				} else {
					NonTerminal(PIECE_POS, result.movesList.push_back(value); , return_Fail; );
					Logic_Check();
				}
				if (str.eof()) { return_Pass; }
				NonTerminal(whitespace, /**/; , return_Fail; );
				state = MOVE;
				break;
			}
		}
		// unreachable, return false just incase
		return_Fail;
	}

	NT(PLAYER_ORDER, playerOrder_t, {}, FIRST, REST)
		case FIRST:
			Terminal('0' <= c&&c <= '9', result.push_back(c-'0'); , return_Fail; );
			state = REST;
			break;
		case REST:
			Terminal('0' <= c&&c <= '9', result.push_back(c-'0'); , return_Pass; );
			break;
	NT_End()

	NT(COLOR_DATA, bool, true, START, REST)
		case START:
			TerminalStr("c:", /**/; , return_Fail; );
			Terminal(('a' <= c&&c <= 'z') || ('A' <= c&&c <= 'Z'), /**/; , return_Fail; );
			state = REST;
			break;
		case REST:
			Terminal(('a' <= c&&c <= 'z') || ('A' <= c&&c <= 'Z'), /**/; , return_Pass; );
			break;
	NT_End()

	NT(PIECE_POS, PlayerMove, {}, START)
		case START:
			NonTerminal(ID, result.id = value; , return_Fail; );
			NonTerminal(whitespace, /**/; , return_Fail; );
			NonTerminal(INT, result.x = value; , return_Fail; );
			NonTerminal(whitespace, /**/; , return_Fail; );
			NonTerminal(INT, result.y = value; , return_Fail; );
			return_Pass;
	NT_End()

	NT(ID, PieceID, {}, START)
		case START:
			NonTerminal(INT, result.num = value-1; , return_Fail; );
			const unsigned num = result.num, numMax = Pieces.size();
			if (!(0 < num&&num <= numMax)) { Logic_Error(); }
			Terminal(c == '.', /**/; , return_Fail; );
			NonTerminal(INT, result.rot = value-1; , return_Fail; );
			const unsigned rot = result.rot, rotMax = Pieces[num].numOptions();
			if (!(0 < rot&&rot <= rotMax)) { Logic_Error(); }
			return_Pass;
	NT_End()

	NT(INT, unsigned, 0, FIRST, REST)
		case FIRST:
			Terminal('0' <= c&&c <= '9', result = c-'0'; , return_Fail; );
			state = REST;
			break;
		case REST:
			Terminal('0' <= c&&c <= '9', result = 10*result + (c-'0'); , return_Pass; );
			break;
	NT_End()

	NT(whitespace, bool, true, FIRST, REST)
		case FIRST:
			Terminal(isWhitespace(c), /**/; , return_Fail; );
			state = REST;
			break;
		case REST:
			Terminal(isWhitespace(c), /**/; , return_Pass; );
			break;
	NT_End()

	#undef return_Fail
	#undef return_Pass
	#undef Next_Char
	#undef Terminal
	#undef TerminalStr
	#undef NonTerminal
	#undef NT
	#undef NT_End
}

enum class fileFormat { CANONICAL };

template <fileFormat format = fileFormat::CANONICAL>
// TODO: change strRaw back into a istringstream
std::optional<BoardHistory> readGame(std::stringstream& strRaw) {
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

	using enum BoardHistory::boardState;
	return parse_CANONICAL_FORMAT(str);
}

void writeGame(BoardHistory&, std::ostringstream& str);
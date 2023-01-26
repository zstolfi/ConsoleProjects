#pragma once
#include "common.hh"

class BoardHistory {

};



#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>
#include <map>
#include <utility> // std::pair
#include <optional>

namespace /*private*/ {
	// using streamPos = std::streampos;
	// using streamOff = std::streamoff;
	using ssRange     = std::pair<std::streampos, std::streampos>;
	using ssRangeList = std::map <std::streampos, std::streampos>;

	constexpr isWhitespace(char c) { return Is_Either(c,' ',',','\r','\n','\t'); }

	namespace parse {
		std::optional<ssRange> whitespace(std::stringstream& str) {
			ssRange range = {str.tellg(), str.tellg()};
			for (char c; str.get(c); ) {
				if (!isWhitespace(c)) { break; }
				range.second += std::streamoff{1};
			}
			if (range.second > range.first) return range; else return std::nullopt;
		}
	}
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
	auto first = whitespaceRanges.begin();
	auto last  = whitespaceRanges.rbegin();
	std::cout << "\tThe first is: " << first->first << " - " << first->second << "\n";
	std::cout << "\tThe last is: "  << last ->first << " - " << last ->second << "\n";

	std::stringstream str{};

	strRaw.clear(); strRaw.seekg(0);
	for (char c; strRaw.get(c); ) {
		std::streampos i = strRaw.tellg() - std::streampos{1};
		if (commentRanges.contains(i)) { strRaw.seekg(commentRanges.at(i)); continue; }
		str.put(c);
		if (strRaw.peek() == EOF) { str << "[EOF]"; }
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
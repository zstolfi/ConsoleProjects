#pragma once
#include "common.hh"

class BoardHistory {

};



#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>
#include <map>
#include <pair>

enum class fileFormat { CANONICAL };

namespace /*private*/ {
	// using streamPos = std::streampos;
	// using streamOff = std::streamoff;
	using ssRange     = std::pair<std::streampos, std::streampos>;
	using ssRangeList = std::map <std::streampos, std::streampos>;

	void iterateRaw(std::istringstream& str, auto&& f) {
		bool cont = true;
		for (char c; str.get(c) && cont; ) {
			if constexpr (std::is_invocable_v<decltype(f),char,bool&>)
				f(c, cont); else f(c);
		}
	}
	// TODO: integrate input string + ignore list into 1 class
	// TODO: add function to said class which cleans up the range data
	void iterateWithIgnore(std::istringstream& str, const ssRangeList& ignoreList, auto&& f) {
		bool cont = true;
		for (char c; cont && str.get(c); ) {
			std::streampos i = str.tellg() - std::streamoff{1};
			if (ignoreList.contains(i)) { str.seekg(ignoreList.at(i)); continue; }

			if constexpr (std::is_invocable_v<decltype(f),char,bool&>)
				f(c, cont); else f(c); // pseudo-break functionality
		}
	}

	namespace parse {
		ssRange whiteSpace(std::istringstream& str, const ssRangeList& ignoreList) {
			ssRange result = {str.tellg(), str.tellg()};
			iterateWithIgnore(str, ignoreList, [&](char c, bool& cont) {
				if (!Is_Either(c,' ',',','\n','\t')) { cont = false; }
			});
			result.second = str.tellg() - std::streamoff{1};
			return result;
		}
	}
}

template <fileFormat format = fileFormat::CANONICAL>
BoardHistory ParseHistory(std::istringstream str) {
	/* preprocess */
	// Ignore comments & leading/trailing whitespaces
	ssRangeList commentRanges{};
	iterateRaw(str, [&](char c) {
		str::streampos i = str.tellg();
		/**/ if (c == '(') { commentRanges[i - std::streamoff{1}]; } // TODO: check for mismatching parenthesis
		else if (c == ')') { commentRanges.rbegin()->second = i; }
	});
	ssRange whitespaceLeading = parse::whitespace(str, commentRanges);

	ssRange wsTest{};
	std::streamoff endOffset = -1;
	do {
		str.clear(); str.seekg(endOffset, std::ios_base::end);
		ssRange wsTest = parse::whiteSpace(str, commentRanges);
	} while(wsTest.second > wsTest.first);
	endOffset += std::streamoff{1};

	str.clear(); str.seekg(endOffset, std::ios_base::end);
	ssRange whitespaceTrailing = parse::whiteSpace(str, commentRanges);

	commentRanges.insert(whitespaceLeading);
	commentRanges.insert(whitespaceTrailing);



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
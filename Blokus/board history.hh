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
		std::optional<ssRange> whitespace(std::istringstream& str, const ssRangeList& ignoreList) {
			ssRange range = {str.tellg(), str.tellg()};
			iterateWithIgnore(str, ignoreList, [&](char c, bool& cont) {
				if (!Is_Either(c,' ',',','\r','\n','\t')) { cont = false; }
			});
			range.second = str.tellg() - std::streamoff{1};
			if (range.second > range.first) { return range; }
			else { return std::nullopt; }
		}
	}
}

template <fileFormat format = fileFormat::CANONICAL>
BoardHistory ParseHistory(std::istringstream& str) {
	std::cout << "Parsing History...\n";
	/* preprocess */
	// Ignore comments & leading/trailing whitespaces
	ssRangeList commentRanges{};
	iterateRaw(str, [&](char c) {
		std::streampos i = str.tellg();
		/**/ if (c == '(') { commentRanges[i - std::streamoff{1}]; } // TODO: check for mismatching parenthesis
		else if (c == ')') { commentRanges.rbegin()->second = i; }
	});

	std::cout << "There are " << commentRanges.size() << " comments.\n";
	for (const auto& [start, end] : commentRanges) {
		std::cout << "\t" << start << " - " << end << "\n";
	}

	str.clear(); str.seekg(0);
	std::optional<ssRange> whitespaceLeading = parse::whitespace(str, commentRanges);
	std::optional<ssRange> whitespaceTrailing = std::nullopt;

	auto lastWhitespace = whitespaceLeading;
	iterateRaw(str, [&](char c) {
		auto maybe_ws = parse::whitespace(str, commentRanges);
		if (maybe_ws) { lastWhitespace = maybe_ws; }
	});
	str.seekg(0, std::ios_base::end);
	std::streampos length = str.tellg();
	if (lastWhitespace->second == length) { whitespaceTrailing = lastWhitespace; }

	if (lastWhitespace) {
		std::cout << "The last whitespace is\n";
		std::cout << "\t" << lastWhitespace->first << " - " << lastWhitespace->second << "\n";
	}

	// std::streamoff endOffset = 0;
	// do {
	// 	endOffset--;
	// 	str.clear(); str.seekg(endOffset, std::ios_base::end);
	// } while (parse::whitespace(str, commentRanges));
	// endOffset++;

	// str.clear(); str.seekg(endOffset, std::ios_base::end);
	// auto whitespaceTrailing = (endOffset == 0) ? std::nullopt : parse::whitespace(str, commentRanges);

	std::cout << "There is" << (whitespaceLeading  ? "" : " no") << " leading whitespace.\n";
	std::cout << "There is" << (whitespaceTrailing ? "" : " no") << " trailing whitespace.\n";

	// commentRanges.insert(whitespaceLeading);
	// commentRanges.insert(whitespaceTrailing);



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
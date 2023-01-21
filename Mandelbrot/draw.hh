#pragma once
#include <map>

// screenType = container of pixType's
// pixType = character & color
template <typename screenType, typename charType, typename colorType, bool squareMode=false>
class Draw {
public:
	Draw(screenType& screen, const unsigned width, const unsigned height ,
	     auto&& applyChar, auto&& applyColor)
	: screen{screen}
	, width{width}, height{height}
	, applyChar{applyChar}, applyColor{applyColor} {}

	screenType& screen;
	const unsigned width;
	const unsigned height;

private:
	using pixType = screenType::value_type;
	using Fchr = void(pixType&, charType);
	using Fcol = void(pixType&, colorType);
	Fchr* applyChar;
	Fcol* applyColor;

public:
	pixType blankPix = {};
	// std::map<std::string, colorType> palette = {};



	void fill(pixType p) { std::fill(screen.begin(), screen.end(), p); }
	void clear() { fill(blankPix); }

	#define Make_Func(NAME,T,VAR,ACTION) \
		void NAME(std::size_t i         , T VAR) { ACTION } \
		void NAME(unsigned x, unsigned y, T VAR) { NAME(y * width + x, VAR); }

	Make_Func(setPixel, pixType, pix, 
		screen[i] = pix;           
	)
	Make_Func(setChar , charType, chr, 
		applyChar (screen[i], chr);
	)
	Make_Func(setColor, colorType, col, 
		applyColor(screen[i], col);
	)
	#undef Make_Func
};

template<typename screenType, typename charType, typename colorType>
using DrawSquare = Draw<screenType, charType, colorType, true>;
#pragma once
#include <map>

// screenType = container of pixType's
// pixType = character & color
// squareMode means treat pairs of chars as a pixel, like so: ██
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
	


	void fill(pixType p) { std::fill(screen.begin(), screen.end(), p); }
	void clear() { fill(blankPix); }



	#define Make_Pixel_Func(NAME,T,VAR) \
	void NAME(std::size_t i, T VAR) { \
		if constexpr (!squareMode) { F(i); } \
		else { \
			i *= 2; \
			if (width&1) i += i/(width-1); /*skip last line when width is odd*/ \
			F(i+0); F(i+1); \
		} \
	} \
	void NAME(unsigned x, unsigned y, T VAR) { \
		NAME(y * (squareMode ? width/2 : width) + x, VAR); \
	}

	#define F(i) screen[i] = pix;
	Make_Pixel_Func(setPixel, pixType, pix);
	#undef F

	#define F(i) applyChar(screen[i], chr);
	Make_Pixel_Func(setChar, charType, chr);
	#undef F

	#define F(i) applyColor(screen[i], col);
	Make_Pixel_Func(setColor, colorType, col);
	#undef F

	#undef Make_Pixel_Func



	struct bounds { unsigned x0, y0, x1, y1; };

	void square(bounds b, pixType p) {
		drawImplicit(b, p, [&](unsigned x, unsigned y) {
			return true;
		});
	}

	void squareBorder(bounds b, pixType p) {
		drawImplicit(b, p, [&](unsigned x, unsigned y) {
			return !(b.x0+1 <= x&&x < b.x1-1)
			    || !(b.y0+1 <= y&&y < b.y1-1);
		});
	}

// private:
	void iterate(bounds b, auto&& f) {
		for (unsigned x=b.x0; x < b.x1; x++) {
		for (unsigned y=b.y0; y < b.y1; y++) {
			f(x,y);
		} }
	}

	void drawImplicit(bounds b, pixType p, auto&& condition) {
		iterate(b, [&](unsigned x, unsigned y) {
			if (condition(x,y)) { setPixel(x,y, p); }
		});
	}
};

template<typename screenType, typename charType, typename colorType>
using DrawSquare = Draw<screenType, charType, colorType, true>;
#pragma once
#include <algorithm>

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
	, Width_Square{squareMode ? width/2 : width}
	, applyChar{applyChar}, applyColor{applyColor} {}

	screenType& screen;
	const unsigned width;
	const unsigned height;

private:
	const unsigned Width_Square;

	using pixType = screenType::value_type;
	using Fchr = void(pixType&, charType);
	using Fcol = void(pixType&, colorType);
	Fchr* applyChar;
	Fcol* applyColor;

public:
	pixType blankPix = {};
	


	void fill(pixType p) { std::fill(screen.begin(), screen.end(), p); }
	void clear() { fill(blankPix); }



	#define Make_Pixel_Func(NAME,T,VAR/*F*/) \
	template <bool squarePix = squareMode> \
	void NAME(std::size_t i, T VAR) { \
		if constexpr (!squarePix) { F(i); } \
		else { \
			i *= 2; \
			if (width&1) i += i/(width-1); /*skip last line when width is odd*/ \
			F(i+0); F(i+1); \
		} \
	} \
	template <bool squarePix = squareMode> \
	void NAME(unsigned x, unsigned y, T VAR) { \
		NAME<squarePix>(y * (squarePix ? Width_Square : width) + x, VAR); \
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



	struct bounds { signed x0, y0, x1, y1; };

	void square(bounds b, pixType p) {
		drawImplicit(b, p, [&](signed x, signed y) {
			return true;
		});
	}

	void squareBorder(bounds b, pixType p) {
		drawImplicit(b, p, [&](signed x, signed y) {
			return !(b.x0+1 <= x&&x < b.x1-1)
			    || !(b.y0+1 <= y&&y < b.y1-1);
		});
	}

	void text(const charType* string) {
		signed x = 0, y = 0;
		for (std::size_t i=0; string[i] != '\0'; i++) {
			charType c = string[i];
			if (c == '\n') { x = 0; y++; break; }
			if ((0 <= x&&x < signed{width}) && (0 <= y&&y < signed{height})) {
				setChar<false>(x,y, c);
			} x++;
		}
	}

// private:
	void iterate(bounds b, auto&& f) {
		// max & min ensure 'f' is only applied inside the screen
		for (signed x = std::max<signed>(0, b.x0); x < std::min<signed>(Width_Square, b.x1); x++) {
		for (signed y = std::max<signed>(0, b.y0); y < std::min<signed>(height      , b.y1); y++) {
			f(x,y);
		} }
	}

	void drawImplicit(bounds b, pixType p, auto&& condition) {
		iterate(b, [&](signed x, signed y) {
			if (condition(x,y)) { setPixel(x,y, p); }
		});
	}

};

template<typename screenType, typename charType, typename colorType>
using DrawSquares = Draw<screenType, charType, colorType, true>;
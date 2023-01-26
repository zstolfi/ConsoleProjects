#pragma once

struct Size {
	unsigned m;
	unsigned n;
};

struct Pos {
	unsigned i;
	unsigned j;
};

struct CropParams {
	signed mStart = 0;
	signed mEnd   = 0;
	signed nStart = 0;
	signed nEnd   = 0;
};

template <typename T, typename... Args>
constexpr bool Is_Either(T&& value, Args&&... args) {
	return ((value == args) || ...); // fold expression
}
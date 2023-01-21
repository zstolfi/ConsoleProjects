#pragma once
#include "common.hh"
#include <concepts>
#include <vector>
#include <cassert>

template <std::integral T=int>
class Matrix {
	std::vector<T> data {};
	Size sizeData = Size {0,0};
	#define m sizeData.m
	#define n sizeData.n

public:
	Matrix(Size size, std::vector<T> list) {
		data = {list};
		sizeData = {size};

		assert(m*n > 0);
		assert(list.size() >= m*n);
	}
	Matrix(Size size) {
		sizeData = {size};
		data.resize(m*n);
		assert(m*n > 0);
	}

	T& operator[](unsigned i, unsigned j) {
		assert(i<m && j<n);
		return data[i * n + j];
	}
	const T& operator[](unsigned i, unsigned j) const {
		assert(i<m && j<n);
		return data[i * n + j];
	}

	const Size size() const {
		return sizeData;
	}

	Matrix<T> crop(CropParams p) {
		Matrix<T> result{{p.mStart + m + p.mEnd ,
		                  p.nStart + n + p.nEnd}};
		result.iterate([&](unsigned i, unsigned j) {
			signed iT = i-p.mStart, jT = j-p.nStart;
			if ((0 <= iT&&iT < (signed)m) && (0 <= jT&&jT < (signed)n)) { result[i,j] = (*this)[iT,jT]; }
		});
		return result;
	}

	void iterate(auto&& f) {
		for (unsigned i=0; i < m; i++) {
		for (unsigned j=0; j < n; j++) {
			f(i,j);
		} }
	}

	// void translate(auto&& f) {
	// 	iterate([&](unsigned i, unsigned j) {
	// 		(*this)[i,j] = f((*this)[i,j]);
	// 	});
	// }

	#undef m
	#undef n
};
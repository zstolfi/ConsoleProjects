#pragma once
#include "common.hh"
#include "symmetry.hh"
#include "matrix.hh"
#include <vector>
#include <string>
#include <algorithm>

class PieceOption {
	Matrix<char> shape;
	Matrix<char> shapeRules;

public:
	PieceOption(Matrix<char> shape) : shape{shape}, shapeRules{generateBorderRules()} {}

	const Matrix<char> getShape() const { return shape; }
	const Matrix<char> getShapeRules() const { return shapeRules; }

private:
	// generate the can-touch / no-touch zones
	// https://files.catbox.moe/j03xx8.png
	Matrix<char> generateBorderRules() {
		Matrix<char> shapeBig = shape.crop({1,1,1,1});
		Matrix<char> result{shapeBig.size()};
		Matrix<char> convolve {{3,3}, {1,0,1 , 0,0,0 , 1,0,1}};

		const auto [m,n] = shapeBig.size();
		shapeBig.iterate([&](unsigned i, unsigned j) {
			bool touched = false;
			int weight = 1;
			convolve.iterate([&](unsigned k, unsigned l) {
				signed iT = i + k -1, jT = j + l -1;
				if ((0 <= iT&&iT < (signed)m) && (0 <= jT&&jT < (signed)n)) {
					bool intersect = shapeBig[iT,jT] > 0;
					if (intersect) { weight *= convolve[k,l]; }
					touched |= intersect;
			} });
			result[i,j] = touched ? (2-weight) : 0 ;
			if (shapeBig[i,j]>0) { result[i,j] = 3; }
		});
		return result;
	}
};

class Piece {
	std::vector<PieceOption> options;
	Group symmetry;

public:
	Piece(Group sym, Size size, std::vector<char> list) : symmetry{sym} {
		Matrix<char> startingShape {size, list};

		for (std::size_t index=0; index < symmetry.size; index++) {
			auto action = symmetry[index];
			Matrix<char> newMat { action.size(size) };
			startingShape.iterate([&](unsigned i, unsigned j) {
				Pos newPos = action.pos(size, {i,j});
				newMat[newPos.i, newPos.j] = startingShape[i,j];
			});

			options.emplace_back(newMat);
		}
	}

	const std::size_t numOptions() const {
		return options.size();
	}

	const PieceOption& getOption(std::size_t index) const {
		return options[index];
	}
};

constexpr std::vector<char> Decode_Str(const std::string& str) {
	std::vector<char> result {};
    for (char c : str) {
        result.push_back((c=='_' || c==' ') ? 0 : 1);
    }
    return result;
}

const std::vector<Piece> Pieces {
	#define Make_Piece(SYMMETRY,M,N,DATA) \
	Piece { Symmetry::SYMMETRY, {M,N}, Decode_Str(DATA) }

	    /* ~~~~~~ 1 Monomino ~~~~~~ */
	Make_Piece(Square   , 1,1, "#") ,
	    /* ~~~~~~ 1 Domino ~~~~~~ */
	Make_Piece(Mirror2  , 2,1, "#"
	                           "#") ,
	    /* ~~~~~~ 2 Trominoes ~~~~~~ */
	Make_Piece(Mirror2  , 3,1, "#"
	                           "#"
	                           "#") ,

	Make_Piece(Mirror1  , 2,2, "# "
	                           "##") ,
	    /* ~~~~~~ 5 Tetrominoes ~~~~~~ */
	Make_Piece(Mirror2  , 4,1, "#"
	                           "#"
	                           "#"
	                           "#") ,

	Make_Piece(None     , 3,2, "# "
	                           "# "
	                           "##") ,

	Make_Piece(Mirror1  , 3,2, "# "
	                           "##"
	                           "# ") ,

	Make_Piece(Rotation2, 3,2, "# "
	                           "##"
	                           " #") ,

	Make_Piece(Square   , 2,2, "##"
	                           "##") ,
	    /* ~~~~~~ 12 Pentominoes ~~~~~~ */
	Make_Piece(Mirror2  , 5,1, "#"
	                           "#"
	                           "#"
	                           "#"
	                           "#") ,

	Make_Piece(None     , 4,2, "# "
	                           "# "
	                           "# "
	                           "##") ,

	Make_Piece(None     , 4,2, "# "
	                           "# "
	                           "##"
	                           "# ") ,

	Make_Piece(None     , 4,2, "# "
	                           "# "
	                           "##"
	                           " #") ,

	Make_Piece(Mirror1  , 3,2, "##"
	                           "# "
	                           "##") ,

	Make_Piece(None     , 3,2, "# "
	                           "##"
	                           "##") ,

	Make_Piece(Mirror1  , 3,3, " # "
	                           " # "
	                           "###") ,

	Make_Piece(Mirror1  , 3,3, "#  "
	                           "#  "
	                           "###") ,

	Make_Piece(Mirror1  , 3,3, "#  "
	                           "## "
	                           " ##") ,

	Make_Piece(Rotation2, 3,3, "#  "
	                           "###"
	                           "  #") ,

	Make_Piece(None     , 3,3, "#  "
	                           "###"
	                           " # ") ,

	Make_Piece(Square   , 3,3, " # "
	                           "###"
	                           " # ") ,
	
	#undef Make_Piece
};
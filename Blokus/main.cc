#include "symmetry.hh"
#include "matrix.hh"
#include "pieces.hh"
#include <iostream>

void printPieceOption(const PieceOption&);
template <typename T> void printMatrix(const Matrix<T>&);
template <typename T> void printMatrix(const Matrix<T>&, auto&& asChar);

int main() {

	std::cout << "start\n\n";

	/*for (std::size_t i=0; i < Pieces.size(); i++) {
		std::cout << "All " << Pieces[i].numOptions() << " rotations of piece #" << (i+1) <<":\n";
		for (std::size_t j=0; j < Pieces[i].numOptions(); j++) {
			std::cout << (i+1) << "." << (j+1) << ":\n";
			printPieceOption(Pieces[i].getOption(j));
			std::cout << "\n";
		}
	}*/

	for (std::size_t i=0; i < Pieces.size(); i++) { std::size_t j = 0;
	// for (std::size_t j=0; j < Pieces[i].numOptions(); j++) {
		const PieceOption& exampleOption = Pieces[i].getOption(j);
		std::cout << (i+1) << "." << (j+1) << "\n";
		std::cout << "shape:\n";
		printPieceOption(exampleOption);
		std::cout << "\nrules:\n";
		printMatrix(exampleOption.getShapeRules(), [](auto n) {
			/**/ if (n == 1) { return '#'; }
			else if (n == 2) { return '.'; }
			else             { return ' '; }
		});
		std::cout << "\n";
	}// }

	/*std::cout << "And now to experiment with making the 20 x 20 board required for Blokus\n\n";

	Matrix<char> board {{20,20}};
	board.iterate([&](unsigned i, unsigned j) {
		board[i,j] = (i%2 ^ j%2) ? '#' : ' ';
	});

	for (unsigned i=0; i < board.size().m; i++) {
		for (unsigned j=0; j < board.size().n; j++) {
			std::cout << board[i,j] << board[i,j];
		}
		std::cout << "\n";
	}*/

	std::cout << "end\n\n";

	return 0;
}



template <typename T>
void printMatrix(const Matrix<T>& matrix, auto&& asChar) {
	for (unsigned i=0; i < matrix.size().m; i++) {
		std::cout << "| ";
		for (unsigned j=0; j < matrix.size().n; j++) {
			std::cout << asChar(matrix[i,j]);
            if (j+1 < matrix.size().n) { std::cout << " "; }
		}
		std::cout << " |\n";
	}
}
template <typename T>
void printMatrix(const Matrix<T>& matrix) {
	return printMatrix(matrix, [](auto n){ return +n; });
}

void printPieceOption(const PieceOption& piece) {
	const auto shape = piece.getShape();
	const auto size = shape.size();

	for (unsigned i=0; i < size.m; i++) {
		std::cout << "| ";
		for (unsigned j=0; j < size.n; j++) {
			std::cout << ((shape[i,j] == 0) ? ' ' : '#');
			if (j+1 < shape.size().n) { std::cout << " "; }
		}
		std::cout << " |\n";
	}
}
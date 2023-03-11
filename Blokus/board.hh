#pragma once
#include "common.hh"
#include "pieces.hh"
#include "matrix.hh"
#include <vector>
#include <variant>

/* internally, all matrix sizes/indexes are y,x; the file format is x,y  */
struct PieceID { unsigned num, rot; };
struct PlayerMove { PieceID id; Pos pos; };
struct NoMove {};

using boardSize_t   = Size;
using numPlayers_t  = unsigned;
using playerOrder_t = std::vector<unsigned>;
using move_t        = std::variant<PlayerMove, NoMove>;
using movesList_t   = std::vector<move_t>;

class Board {
	/* "scratch" board, only used for internal checks */
	Matrix<char> board;

public:
	// TODO: make boardSize part of the grammar
	boardSize_t boardSize/* = {20, 20}*/;
	numPlayers_t numPlayers;
	playerOrder_t playerOrder;
	// player color is not needed by the computer
	movesList_t movesList;

	explicit Board(boardSize_t bs) : board{bs}, boardSize{bs} {
		const char EMPTY = -1;
		board.fill(EMPTY);
	}

	Matrix<char>& render() {
		// getValidity();
		return board;
	}

	// thanks Bisqwit
	#define Validity_List(o) \
		o(VALID) \
		o(NO_PLAYERS) o(TOO_MANY_PLAYERS) o(PLAYER_ORDER_COUNT) \
		o(TOO_MANY_MOVES) \
		o(EARLY_MOVE_SKIP) o(MOVE_AFTER_FINAL) \
		o(PIECE_OUT_OF_BOUNDS) o(OVERLAPPING_TILES) /*o(TOUCHING_EDGES)*/

	#define o(n) n,
	enum validity { Validity_List(o) };
	#undef o

	validity getValidity() {
		/* game checking logic */
		if (numPlayers <= 0) { return NO_PLAYERS; }
		if (numPlayers >  4) { return TOO_MANY_PLAYERS; }
		if (playerOrder.size() != numPlayers) { return PLAYER_ORDER_COUNT; }
		if (movesList.size() > Pieces.size()*numPlayers) { return TOO_MANY_MOVES; }

		/* spatial checks, unoptimized */
		const char EMPTY = -1;
		board.fill(EMPTY);
		for (unsigned moveNumber=0; moveNumber < movesList.size(); moveNumber++) {
			// player's turn may be a move, or no move
			const auto& turn = movesList[moveNumber];
			const char player = moveNumber % numPlayers;
			bool movesAvailable = false;
			/* logic, assume true for now */
			movesAvailable = true;
			// /**/ if (std::holds_alternative<NoMove>(turn)     &&  movesAvailable) { return EARLY_MOVE_SKIP; }
			// else if (std::holds_alternative<PlayerMove>(turn) && !movesAvailable) { return MOVE_AFTER_FINAL; }
			if (std::holds_alternative<PlayerMove>(turn)) {
				const auto& move = std::get<PlayerMove>(turn);
				const auto& curPiece = Pieces[move.id.num].getOption(move.id.rot);
				const auto& curShape = curPiece.getShape();
				/* bounds check */
				if (move.pos.i < 0 || move.pos.i + curShape.size().m > boardSize.m
				 || move.pos.j < 0 || move.pos.j + curShape.size().n > boardSize.n) {
					return PIECE_OUT_OF_BOUNDS;
				}

				/* overlap check (no edge check yet) */
				validity pieceValidity = VALID;
				curShape.iterate_const([&](unsigned i, unsigned j) {
					if (curShape[i,j]) {
						char& tile = board[i + move.pos.i, j + move.pos.j];
						if (tile == EMPTY) { tile = player; }
						else { pieceValidity = OVERLAPPING_TILES; return; }
					}
				});
			}
		}

		return VALID;
	}

};
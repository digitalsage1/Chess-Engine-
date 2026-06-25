#pragma once
#include "board.h"
#include "movegen.h"

// Standard material point values used for board evaluation.
// King is intentionally very high so the AI never "trades" it away in evaluation math
// (actual king safety/checkmate is handled separately by the legal move generator).
constexpr int PAWN_VALUE = 1;
constexpr int KNIGHT_VALUE = 3;
constexpr int BISHOP_VALUE = 3;
constexpr int ROOK_VALUE = 5;
constexpr int QUEEN_VALUE = 9;
constexpr int KING_VALUE = 1000;

// Evaluates the board from White's perspective: positive = good for White,
// negative = good for Black. Pure material count, no positional bonuses (kept
// simple on purpose for a "not too smart" AI).
int EvaluateBoard(const Board& board);

// Picks a move for `aiColor` to play.
// randomMoveChance is 0.0-1.0 (e.g. 0.3 = 30% chance of playing a random legal move
// instead of the minimax-selected best move) - this is what keeps the AI beatable.
Move ChooseAiMove(const Board& board, PieceColor aiColor, float randomMoveChance, int searchDepth);
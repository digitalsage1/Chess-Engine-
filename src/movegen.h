#pragma once
#include "board.h"
#include <vector>

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;

    // Special move flags - false/None by default for ordinary moves
    bool isCastleKingside = false;
    bool isCastleQueenside = false;
    bool isEnPassantCapture = false;
    bool isPromotion = false; // always promotes to Queen for simplicity
};

// Returns true if (row, col) is inside the 8x8 board
bool IsInBounds(int row, int col);

// Generates pseudo-legal moves for the piece at (row, col).
// "Pseudo-legal" = follows piece movement rules, but does NOT yet check
// whether the move would leave your own king in check (that comes later).
std::vector<Move> GeneratePseudoLegalMoves(const Board& board, int row, int col);

// Returns true if (row, col) is attacked by any piece belonging to attackerColor.
// Used to detect check: e.g. IsSquareAttacked(board, kingRow, kingCol, enemyColor)
bool IsSquareAttacked(const Board& board, int row, int col, PieceColor attackerColor);

// Returns true if the king belonging to kingColor is currently in check.
bool IsKingInCheck(const Board& board, PieceColor kingColor);

// Returns only the truly legal moves for the piece at (row, col) -
// i.e. pseudo-legal moves that do NOT leave the mover's own king in check.
std::vector<Move> GenerateLegalMoves(const Board& board, int row, int col);

// Returns true if the given color has at least one legal move anywhere on the board.
// Used later for checkmate/stalemate detection.
bool HasAnyLegalMoves(const Board& board, PieceColor color);


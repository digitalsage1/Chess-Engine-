#pragma once
#include "board.h"
#include "movegen.h"
#include <string>
#include <vector>

// One entry in the move history log, already formatted as plain English text
// ready to display (e.g. "Knight to F3", "Pawn captures D5").
struct MoveLogEntry {
    std::string text;
    PieceColor playerColor;
};

// Tracks everything needed for the UI on top of the core Board: clocks, names,
// move history, and overall match outcome.
struct GameState {
    std::string whiteName = "Player 1";
    std::string blackName = "Player 2";

    // Chess clock, counting DOWN from the starting allotment, in seconds.
    float whiteTimeRemaining = 5.0f * 60.0f;
    float blackTimeRemaining = 5.0f * 60.0f;

    std::vector<MoveLogEntry> moveLog;

    bool gameOver = false;
    std::string resultText; // e.g. "Checkmate! Player 1 wins" - set once gameOver becomes true
};

// Formats seconds as M:SS for clock display (e.g. 247.0f -> "4:07").
std::string FormatClockTime(float secondsRemaining);

// Converts a board column (0-7) to its algebraic file letter ('a'-'h').
char ColToFile(int col);

// Converts a board row (0-7) to its algebraic rank number ('1'-'8', row 7 = rank 1).
char RowToRank(int row);

// Returns the plain-English name of a piece type (e.g. "Knight", "Pawn").
std::string PieceTypeName(PieceType type);

// Converts a move into a plain-English description for the move log,
// e.g. "Knight to F3", "Pawn captures on D5", "Castles kingside".
// Must be called BEFORE the move is applied to the board, since it needs
// to inspect what piece is moving and what (if anything) is being captured.
std::string DescribeMove(const Board& boardBeforeMove, const Move& move);

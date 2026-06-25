#include "gamestate.h"
#include <cstdio>

std::string FormatClockTime(float secondsRemaining) {
    if (secondsRemaining < 0) secondsRemaining = 0;
    int totalSeconds = (int)secondsRemaining;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d:%02d", minutes, seconds);
    return std::string(buffer);
}

char ColToFile(int col) {
    return (char)('a' + col);
}

char RowToRank(int row) {
    // Row 7 (bottom of array) = rank 1, row 0 (top of array) = rank 8
    return (char)('1' + (7 - row));
}

std::string PieceTypeName(PieceType type) {
    switch (type) {
    case PieceType::Pawn:   return "Pawn";
    case PieceType::Knight: return "Knight";
    case PieceType::Bishop: return "Bishop";
    case PieceType::Rook:   return "Rook";
    case PieceType::Queen:  return "Queen";
    case PieceType::King:   return "King";
    default:                return "";
    }
}

std::string DescribeMove(const Board& boardBeforeMove, const Move& move) {
    const Piece& movingPiece = boardBeforeMove.squares[move.fromRow][move.fromCol];
    const Piece& targetPiece = boardBeforeMove.squares[move.toRow][move.toCol];

    bool isCapture = (targetPiece.type != PieceType::None) || move.isEnPassantCapture;

    std::string destSquare = std::string(1, ColToFile(move.toCol)) + std::string(1, RowToRank(move.toRow));

    if (move.isCastleKingside) return "Castles kingside";
    if (move.isCastleQueenside) return "Castles queenside";

    std::string pieceName = PieceTypeName(movingPiece.type);
    std::string result;

    if (isCapture) {
        result = pieceName + " captures on " + destSquare;
    }
    else {
        result = pieceName + " to " + destSquare;
    }

    if (move.isPromotion) {
        result += " (promotes to Queen)";
    }

    return result;
}

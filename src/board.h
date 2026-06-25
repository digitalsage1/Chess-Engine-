#pragma once

enum class PieceType { None, Pawn, Knight, Bishop, Rook, Queen, King };
enum class PieceColor { None, White, Black };

struct Piece {
    PieceType type = PieceType::None;
    PieceColor color = PieceColor::None;
};

class Board {
public:
    Piece squares[8][8];
    PieceColor turn = PieceColor::White;

    // Castling rights - true means that piece has NOT moved yet (still eligible to castle)
    bool whiteKingMoved = false;
    bool whiteKingsideRookMoved = false;
    bool whiteQueensideRookMoved = false;
    bool blackKingMoved = false;
    bool blackKingsideRookMoved = false;
    bool blackQueensideRookMoved = false;

    // En passant target square - the square a pawn could capture into via en passant
    // right now. Set to -1, -1 if en passant is not currently available.
    int enPassantTargetRow = -1;
    int enPassantTargetCol = -1;

    Board();
    void SetupStartingPosition();
};

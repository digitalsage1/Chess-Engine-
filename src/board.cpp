#include "board.h"

Board::Board() {
    SetupStartingPosition();
}

void Board::SetupStartingPosition() {
    // Clear everything first
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            squares[row][col] = Piece{ PieceType::None, PieceColor::None };
        }
    }

    // Pawns
    for (int col = 0; col < 8; col++) {
        squares[1][col] = Piece{ PieceType::Pawn, PieceColor::Black };
        squares[6][col] = Piece{ PieceType::Pawn, PieceColor::White };
    }

    // Back rank piece order: Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook
    PieceType backRank[8] = {
        PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen,
        PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook
    };

    for (int col = 0; col < 8; col++) {
        squares[0][col] = Piece{ backRank[col], PieceColor::Black };
        squares[7][col] = Piece{ backRank[col], PieceColor::White };
    }

    turn = PieceColor::White;
}
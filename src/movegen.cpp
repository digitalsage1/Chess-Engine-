#include "movegen.h"

// Forward declaration - defined further down, but GenerateKingMoves needs it for castling checks
bool IsSquareAttacked(const Board& board, int row, int col, PieceColor attackerColor);

bool IsInBounds(int row, int col) {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

// Helper: adds a move if the destination is empty or has an enemy piece (capture)
static void TryAddMove(const Board& board, std::vector<Move>& moves,
    int fromRow, int fromCol, int toRow, int toCol, PieceColor movingColor) {
    if (!IsInBounds(toRow, toCol)) return;

    const Piece& target = board.squares[toRow][toCol];
    if (target.color == movingColor) return; // can't capture your own piece

    moves.push_back({ fromRow, fromCol, toRow, toCol });
}

static std::vector<Move> GeneratePawnMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    const Piece& pawn = board.squares[row][col];

    // White moves "up" the array (toward row 0), Black moves "down" (toward row 7)
    int direction = (pawn.color == PieceColor::White) ? -1 : 1;
    int startRow = (pawn.color == PieceColor::White) ? 6 : 1;
    int promotionRow = (pawn.color == PieceColor::White) ? 0 : 7;

    int oneStepRow = row + direction;

    // Forward move (only if square is empty — pawns can't capture straight ahead)
    if (IsInBounds(oneStepRow, col) && board.squares[oneStepRow][col].type == PieceType::None) {
        Move forward{ row, col, oneStepRow, col };
        forward.isPromotion = (oneStepRow == promotionRow);
        moves.push_back(forward);

        // Two-step move from starting row (only if both squares ahead are empty)
        int twoStepRow = row + 2 * direction;
        if (row == startRow && board.squares[twoStepRow][col].type == PieceType::None) {
            moves.push_back({ row, col, twoStepRow, col });
        }
    }

    // Diagonal captures
    for (int dc : {-1, 1}) {
        int captureCol = col + dc;
        if (IsInBounds(oneStepRow, captureCol)) {
            const Piece& target = board.squares[oneStepRow][captureCol];
            if (target.type != PieceType::None && target.color != pawn.color) {
                Move capture{ row, col, oneStepRow, captureCol };
                capture.isPromotion = (oneStepRow == promotionRow);
                moves.push_back(capture);
            }

            // En passant: if the en passant target matches this diagonal square,
            // we can capture even though that square itself is currently empty.
            if (oneStepRow == board.enPassantTargetRow && captureCol == board.enPassantTargetCol) {
                Move epMove{ row, col, oneStepRow, captureCol };
                epMove.isEnPassantCapture = true;
                moves.push_back(epMove);
            }
        }
    }

    return moves;
}

// Slides repeatedly in one (dRow, dCol) direction until blocked or off-board.
// Used by Bishop, Rook, and Queen since they all "slide" - only the directions differ.
static void SlideInDirection(const Board& board, std::vector<Move>& moves,
    int row, int col, int dRow, int dCol, PieceColor movingColor) {
    int toRow = row + dRow;
    int toCol = col + dCol;

    while (IsInBounds(toRow, toCol)) {
        const Piece& target = board.squares[toRow][toCol];

        if (target.type == PieceType::None) {
            // Empty square - can move here, and keep sliding further
            moves.push_back({ row, col, toRow, toCol });
        }
        else {
            // Occupied square - if enemy, can capture; either way, stop sliding here
            if (target.color != movingColor) {
                moves.push_back({ row, col, toRow, toCol });
            }
            break;
        }

        toRow += dRow;
        toCol += dCol;
    }
}

static std::vector<Move> GenerateBishopMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    PieceColor color = board.squares[row][col].color;

    // Bishops move diagonally in all 4 directions
    const int directions[4][2] = { {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };
    for (auto& d : directions) {
        SlideInDirection(board, moves, row, col, d[0], d[1], color);
    }
    return moves;
}

static std::vector<Move> GenerateRookMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    PieceColor color = board.squares[row][col].color;

    // Rooks move horizontally/vertically in 4 directions
    const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
    for (auto& d : directions) {
        SlideInDirection(board, moves, row, col, d[0], d[1], color);
    }
    return moves;
}

static std::vector<Move> GenerateQueenMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    PieceColor color = board.squares[row][col].color;

    // Queen = bishop + rook directions combined (all 8 directions)
    const int directions[8][2] = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1},
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };
    for (auto& d : directions) {
        SlideInDirection(board, moves, row, col, d[0], d[1], color);
    }
    return moves;
}

static std::vector<Move> GenerateKingMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    const Piece& king = board.squares[row][col];

    // King moves one square in any of the 8 surrounding directions
    const int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };

    for (auto& d : directions) {
        int toRow = row + d[0];
        int toCol = col + d[1];
        TryAddMove(board, moves, row, col, toRow, toCol, king.color);
    }

    // ---- Castling ----
    // Conditions for either side: king and that rook haven't moved, squares between
    // them are empty, and the king is not currently in check, doesn't pass through
    // an attacked square, and doesn't land on an attacked square.
    bool isWhite = (king.color == PieceColor::White);
    int homeRow = isWhite ? 7 : 0;

    if (row == homeRow && col == 4) { // king must be on its original square
        bool kingMoved = isWhite ? board.whiteKingMoved : board.blackKingMoved;

        if (!kingMoved) {
            PieceColor enemyColor = isWhite ? PieceColor::Black : PieceColor::White;

            // Kingside (short castle): rook on col 7, king ends on col 6, rook ends on col 5
            bool kingsideRookMoved = isWhite ? board.whiteKingsideRookMoved : board.blackKingsideRookMoved;
            const Piece& kingsideRook = board.squares[homeRow][7];
            if (!kingsideRookMoved && kingsideRook.type == PieceType::Rook && kingsideRook.color == king.color
                && board.squares[homeRow][5].type == PieceType::None
                && board.squares[homeRow][6].type == PieceType::None
                && !IsSquareAttacked(board, homeRow, 4, enemyColor)
                && !IsSquareAttacked(board, homeRow, 5, enemyColor)
                && !IsSquareAttacked(board, homeRow, 6, enemyColor)) {
                Move castleK{ row, col, homeRow, 6 };
                castleK.isCastleKingside = true;
                moves.push_back(castleK);
            }

            // Queenside (long castle): rook on col 0, king ends on col 2, rook ends on col 3
            bool queensideRookMoved = isWhite ? board.whiteQueensideRookMoved : board.blackQueensideRookMoved;
            const Piece& queensideRook = board.squares[homeRow][0];
            if (!queensideRookMoved && queensideRook.type == PieceType::Rook && queensideRook.color == king.color
                && board.squares[homeRow][1].type == PieceType::None
                && board.squares[homeRow][2].type == PieceType::None
                && board.squares[homeRow][3].type == PieceType::None
                && !IsSquareAttacked(board, homeRow, 4, enemyColor)
                && !IsSquareAttacked(board, homeRow, 3, enemyColor)
                && !IsSquareAttacked(board, homeRow, 2, enemyColor)) {
                Move castleQ{ row, col, homeRow, 2 };
                castleQ.isCastleQueenside = true;
                moves.push_back(castleQ);
            }
        }
    }

    return moves;
}

static std::vector<Move> GenerateKnightMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    const Piece& knight = board.squares[row][col];

    // All 8 possible "L-shaped" knight offsets
    const int offsets[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        { 1, -2}, { 1, 2}, { 2, -1}, { 2, 1}
    };

    for (auto& offset : offsets) {
        int toRow = row + offset[0];
        int toCol = col + offset[1];
        TryAddMove(board, moves, row, col, toRow, toCol, knight.color);
    }

    return moves;
}

std::vector<Move> GeneratePseudoLegalMoves(const Board& board, int row, int col) {
    const Piece& piece = board.squares[row][col];

    switch (piece.type) {
    case PieceType::Pawn:
        return GeneratePawnMoves(board, row, col);
    case PieceType::Knight:
        return GenerateKnightMoves(board, row, col);
    case PieceType::Bishop:
        return GenerateBishopMoves(board, row, col);
    case PieceType::Rook:
        return GenerateRookMoves(board, row, col);
    case PieceType::Queen:
        return GenerateQueenMoves(board, row, col);
    case PieceType::King:
        return GenerateKingMoves(board, row, col);
    default:
        return {};
    }
}

// Finds the (row, col) of the king belonging to kingColor.
// Returns {-1, -1} if somehow no king is found (shouldn't happen in a real game).
static void FindKing(const Board& board, PieceColor kingColor, int& outRow, int& outCol) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            const Piece& p = board.squares[row][col];
            if (p.type == PieceType::King && p.color == kingColor) {
                outRow = row;
                outCol = col;
                return;
            }
        }
    }
    outRow = -1;
    outCol = -1;
}

bool IsSquareAttacked(const Board& board, int row, int col, PieceColor attackerColor) {
    // Check every square on the board - if it holds a piece of attackerColor,
    // see if that piece's pseudo-legal moves include (row, col).
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            const Piece& p = board.squares[r][c];
            if (p.color != attackerColor) continue;

            std::vector<Move> moves = GeneratePseudoLegalMoves(board, r, c);
            for (const Move& m : moves) {
                if (m.toRow == row && m.toCol == col) return true;
            }
        }
    }
    return false;
}

bool IsKingInCheck(const Board& board, PieceColor kingColor) {
    int kingRow, kingCol;
    FindKing(board, kingColor, kingRow, kingCol);
    if (kingRow == -1) return false; // no king found - shouldn't normally happen

    PieceColor enemyColor = (kingColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    return IsSquareAttacked(board, kingRow, kingCol, enemyColor);
}

// Returns a copy of the board with the given move applied (does not touch turn tracking,
// castling rights, or en passant target - this is purely for "what if" check simulation,
// not real gameplay moves, but it DOES need to handle en passant/castling piece movement
// correctly so check detection on those special moves is accurate).
static Board SimulateMove(const Board& board, const Move& move) {
    Board copy = board;
    const Piece movingPiece = copy.squares[move.fromRow][move.fromCol];

    if (move.isEnPassantCapture) {
        copy.squares[move.fromRow][move.toCol] = Piece{ PieceType::None, PieceColor::None };
    }

    copy.squares[move.toRow][move.toCol] = movingPiece;
    copy.squares[move.fromRow][move.fromCol] = Piece{ PieceType::None, PieceColor::None };

    if (move.isCastleKingside) {
        int homeRow = move.fromRow;
        copy.squares[homeRow][5] = copy.squares[homeRow][7];
        copy.squares[homeRow][7] = Piece{ PieceType::None, PieceColor::None };
    }
    else if (move.isCastleQueenside) {
        int homeRow = move.fromRow;
        copy.squares[homeRow][3] = copy.squares[homeRow][0];
        copy.squares[homeRow][0] = Piece{ PieceType::None, PieceColor::None };
    }

    return copy;
}

std::vector<Move> GenerateLegalMoves(const Board& board, int row, int col) {
    std::vector<Move> legalMoves;
    PieceColor movingColor = board.squares[row][col].color;
    if (movingColor == PieceColor::None) return legalMoves;

    std::vector<Move> pseudoLegal = GeneratePseudoLegalMoves(board, row, col);

    for (const Move& m : pseudoLegal) {
        Board afterMove = SimulateMove(board, m);
        if (!IsKingInCheck(afterMove, movingColor)) {
            legalMoves.push_back(m);
        }
        // If the king WOULD be in check after this move, we simply don't add it -
        // this is what filters out "moving into check" and "ignoring an existing check"
    }

    return legalMoves;
}

bool HasAnyLegalMoves(const Board& board, PieceColor color) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board.squares[row][col].color != color) continue;
            std::vector<Move> moves = GenerateLegalMoves(board, row, col);
            if (!moves.empty()) return true;
        }
    }
    return false;
}




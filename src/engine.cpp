#include "engine.h"
#include <vector>
#include <cstdlib>
#include <limits>

static int PieceValue(PieceType type) {
    switch (type) {
    case PieceType::Pawn:   return PAWN_VALUE;
    case PieceType::Knight: return KNIGHT_VALUE;
    case PieceType::Bishop: return BISHOP_VALUE;
    case PieceType::Rook:   return ROOK_VALUE;
    case PieceType::Queen:  return QUEEN_VALUE;
    case PieceType::King:   return KING_VALUE;
    default:                return 0;
    }
}

int EvaluateBoard(const Board& board) {
    int score = 0;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            const Piece& p = board.squares[row][col];
            if (p.type == PieceType::None) continue;

            int value = PieceValue(p.type);
            score += (p.color == PieceColor::White) ? value : -value;
        }
    }
    return score;
}

// Collects every legal move for every piece belonging to `color`.
static std::vector<Move> GenerateAllLegalMoves(const Board& board, PieceColor color) {
    std::vector<Move> allMoves;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board.squares[row][col].color != color) continue;
            std::vector<Move> pieceMoves = GenerateLegalMoves(board, row, col);
            allMoves.insert(allMoves.end(), pieceMoves.begin(), pieceMoves.end());
        }
    }
    return allMoves;
}

// Applies a move to a board copy and returns the result. Mirrors main.cpp's ApplyMove
// logic (castling, en passant, promotion, turn switching) since the AI's search needs
// to simulate full real moves, not just piece relocation.
static Board ApplyMoveCopy(const Board& board, const Move& move) {
    Board copy = board;
    const Piece movingPiece = copy.squares[move.fromRow][move.fromCol];

    if (move.isEnPassantCapture) {
        copy.squares[move.fromRow][move.toCol] = Piece{ PieceType::None, PieceColor::None };
    }

    copy.squares[move.toRow][move.toCol] = movingPiece;
    copy.squares[move.fromRow][move.fromCol] = Piece{ PieceType::None, PieceColor::None };

    if (move.isPromotion) {
        copy.squares[move.toRow][move.toCol].type = PieceType::Queen;
    }

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

    if (movingPiece.type == PieceType::King) {
        if (movingPiece.color == PieceColor::White) copy.whiteKingMoved = true;
        else copy.blackKingMoved = true;
    }
    if (movingPiece.type == PieceType::Rook) {
        if (move.fromRow == 7 && move.fromCol == 0) copy.whiteQueensideRookMoved = true;
        if (move.fromRow == 7 && move.fromCol == 7) copy.whiteKingsideRookMoved = true;
        if (move.fromRow == 0 && move.fromCol == 0) copy.blackQueensideRookMoved = true;
        if (move.fromRow == 0 && move.fromCol == 7) copy.blackKingsideRookMoved = true;
    }
    if (move.toRow == 7 && move.toCol == 0) copy.whiteQueensideRookMoved = true;
    if (move.toRow == 7 && move.toCol == 7) copy.whiteKingsideRookMoved = true;
    if (move.toRow == 0 && move.toCol == 0) copy.blackQueensideRookMoved = true;
    if (move.toRow == 0 && move.toCol == 7) copy.blackKingsideRookMoved = true;

    copy.enPassantTargetRow = -1;
    copy.enPassantTargetCol = -1;
    if (movingPiece.type == PieceType::Pawn) {
        int rowDiff = move.toRow - move.fromRow;
        if (rowDiff == 2 || rowDiff == -2) {
            copy.enPassantTargetRow = (move.fromRow + move.toRow) / 2;
            copy.enPassantTargetCol = move.fromCol;
        }
    }

    copy.turn = (copy.turn == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    return copy;
}

// Standard minimax: White tries to maximize EvaluateBoard, Black tries to minimize it.
// `depth` counts down to 0, at which point we just return the static evaluation.
static int Minimax(const Board& board, int depth, PieceColor colorToMove) {
    if (depth == 0) {
        return EvaluateBoard(board);
    }

    std::vector<Move> moves = GenerateAllLegalMoves(board, colorToMove);

    if (moves.empty()) {
        // No legal moves: either checkmate or stalemate.
        bool inCheck = IsKingInCheck(board, colorToMove);
        if (inCheck) {
            // Checkmate - very bad for whoever is checkmated, very good for the other side.
            return (colorToMove == PieceColor::White) ? -100000 : 100000;
        }
        return 0; // stalemate = neutral/draw
    }

    PieceColor nextColor = (colorToMove == PieceColor::White) ? PieceColor::Black : PieceColor::White;

    if (colorToMove == PieceColor::White) {
        int best = std::numeric_limits<int>::min();
        for (const Move& m : moves) {
            Board after = ApplyMoveCopy(board, m);
            int score = Minimax(after, depth - 1, nextColor);
            if (score > best) best = score;
        }
        return best;
    }
    else {
        int best = std::numeric_limits<int>::max();
        for (const Move& m : moves) {
            Board after = ApplyMoveCopy(board, m);
            int score = Minimax(after, depth - 1, nextColor);
            if (score < best) best = score;
        }
        return best;
    }
}

static float RandomFloat0to1() {
    return (float)rand() / (float)RAND_MAX;
}

Move ChooseAiMove(const Board& board, PieceColor aiColor, float randomMoveChance, int searchDepth) {
    std::vector<Move> allMoves = GenerateAllLegalMoves(board, aiColor);

    // Should not normally be called with no legal moves (that's checkmate/stalemate,
    // checked by the caller), but guard anyway to avoid undefined behavior.
    if (allMoves.empty()) {
        return Move{ -1, -1, -1, -1 };
    }

    // "Not too smart": sometimes just play a random legal move instead of thinking.
    if (RandomFloat0to1() < randomMoveChance) {
        int randomIndex = rand() % allMoves.size();
        return allMoves[randomIndex];
    }

    // Otherwise, evaluate each candidate move with minimax and pick the best one
    // from aiColor's perspective.
    PieceColor opponentColor = (aiColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;

    Move bestMove = allMoves[0];
    int bestScore = (aiColor == PieceColor::White)
        ? std::numeric_limits<int>::min()
        : std::numeric_limits<int>::max();

    for (const Move& m : allMoves) {
        Board after = ApplyMoveCopy(board, m);
        int score = Minimax(after, searchDepth - 1, opponentColor);

        bool better = (aiColor == PieceColor::White) ? (score > bestScore) : (score < bestScore);
        if (better) {
            bestScore = score;
            bestMove = m;
        }
    }

    return bestMove;
}

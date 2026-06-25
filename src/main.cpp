#include "raylib.h"
#include "board.h"
#include "movegen.h"
#include "engine.h"
#include "gamestate.h"
#include "gui.h"
#include <vector>
#include <string>

enum class AppState { NameEntry, Menu, Playing };

using namespace Layout;

// ---- Coordinate + game logic helpers ----
bool ScreenToBoard(Vector2 mousePos, int& outRow, int& outCol) {
    float relX = mousePos.x - BoardOriginX();
    float relY = mousePos.y - BoardOriginY();
    outCol = (int)(relX / SQUARE_SIZE);
    outRow = (int)(relY / SQUARE_SIZE);
    if (relX < 0 || relY < 0) return false;
    return IsInBounds(outRow, outCol);
}

bool IsLegalDestination(const std::vector<Move>& moves, int row, int col) {
    for (const Move& m : moves) {
        if (m.toRow == row && m.toCol == col) return true;
    }
    return false;
}

void ApplyMove(Board& board, const Move& move) {
    const Piece movingPiece = board.squares[move.fromRow][move.fromCol];

    if (move.isEnPassantCapture) {
        board.squares[move.fromRow][move.toCol] = Piece{ PieceType::None, PieceColor::None };
    }

    board.squares[move.toRow][move.toCol] = movingPiece;
    board.squares[move.fromRow][move.fromCol] = Piece{ PieceType::None, PieceColor::None };

    if (move.isPromotion) {
        board.squares[move.toRow][move.toCol].type = PieceType::Queen;
    }

    if (move.isCastleKingside) {
        int homeRow = move.fromRow;
        board.squares[homeRow][5] = board.squares[homeRow][7];
        board.squares[homeRow][7] = Piece{ PieceType::None, PieceColor::None };
    }
    else if (move.isCastleQueenside) {
        int homeRow = move.fromRow;
        board.squares[homeRow][3] = board.squares[homeRow][0];
        board.squares[homeRow][0] = Piece{ PieceType::None, PieceColor::None };
    }

    if (movingPiece.type == PieceType::King) {
        if (movingPiece.color == PieceColor::White) board.whiteKingMoved = true;
        else board.blackKingMoved = true;
    }
    if (movingPiece.type == PieceType::Rook) {
        if (move.fromRow == 7 && move.fromCol == 0) board.whiteQueensideRookMoved = true;
        if (move.fromRow == 7 && move.fromCol == 7) board.whiteKingsideRookMoved = true;
        if (move.fromRow == 0 && move.fromCol == 0) board.blackQueensideRookMoved = true;
        if (move.fromRow == 0 && move.fromCol == 7) board.blackKingsideRookMoved = true;
    }
    if (move.toRow == 7 && move.toCol == 0) board.whiteQueensideRookMoved = true;
    if (move.toRow == 7 && move.toCol == 7) board.whiteKingsideRookMoved = true;
    if (move.toRow == 0 && move.toCol == 0) board.blackQueensideRookMoved = true;
    if (move.toRow == 0 && move.toCol == 7) board.blackKingsideRookMoved = true;

    board.enPassantTargetRow = -1;
    board.enPassantTargetCol = -1;
    if (movingPiece.type == PieceType::Pawn) {
        int rowDiff = move.toRow - move.fromRow;
        if (rowDiff == 2 || rowDiff == -2) {
            board.enPassantTargetRow = (move.fromRow + move.toRow) / 2;
            board.enPassantTargetCol = move.fromCol;
        }
    }

    board.turn = (board.turn == PieceColor::White) ? PieceColor::Black : PieceColor::White;
}

// Performs a move AND records it in the game log + checks for game-over conditions.
void PerformMoveWithLogging(Board& board, GameState& state, const Move& move) {
    std::string description = DescribeMove(board, move);
    PieceColor mover = board.turn;

    ApplyMove(board, move);

    state.moveLog.push_back({ description, mover });

    bool nextPlayerInCheck = IsKingInCheck(board, board.turn);
    bool nextPlayerHasMoves = HasAnyLegalMoves(board, board.turn);

    if (!nextPlayerHasMoves) {
        state.gameOver = true;
        if (nextPlayerInCheck) {
            std::string winnerName = (board.turn == PieceColor::White) ? state.blackName : state.whiteName;
            state.resultText = "Checkmate! " + winnerName + " wins";
        }
        else {
            state.resultText = "Stalemate - Draw";
        }
    }
}

void ResetGameState(Board& board, GameState& state, bool& pieceSelected, std::vector<Move>& legalMoves) {
    board = Board();
    state.whiteTimeRemaining = 5.0f * 60.0f;
    state.blackTimeRemaining = 5.0f * 60.0f;
    state.moveLog.clear();
    state.gameOver = false;
    state.resultText.clear();
    pieceSelected = false;
    legalMoves.clear();
}

int main() {
    InitWindow(WindowWidth(), WindowHeight(), "Chess Engine");
    SetWindowPosition(50, 30);
    SetTargetFPS(60);

    LoadPieceTextures();

    AppState appState = AppState::NameEntry;
    GameMode mode = GameMode::None;
    PieceColor aiColor = PieceColor::Black;

    const float AI_RANDOM_MOVE_CHANCE = 0.25f;
    const int AI_SEARCH_DEPTH = 2;

    std::string whiteNameInput, blackNameInput;
    int activeField = 1;

    Board board;
    GameState state;

    bool pieceSelected = false;
    int selectedRow = -1, selectedCol = -1;
    std::vector<Move> legalMovesForSelected;

    while (!WindowShouldClose()) {

        // ---- Name entry screen ----
        if (appState == AppState::NameEntry) {
            BeginDrawing();
            bool done = DrawNameEntryScreen(whiteNameInput, blackNameInput, activeField);
            EndDrawing();

            if (done) {
                state.whiteName = whiteNameInput.empty() ? "Player 1" : whiteNameInput;
                state.blackName = blackNameInput.empty() ? "Player 2" : blackNameInput;
                appState = AppState::Menu;
            }
            continue;
        }

        // ---- Mode select menu ----
        if (appState == AppState::Menu) {
            BeginDrawing();
            GameMode chosen = DrawModeMenu();
            EndDrawing();

            if (chosen != GameMode::None) {
                mode = chosen;
                if (mode == GameMode::HumanVsAi && state.blackName.find("(AI)") == std::string::npos) {
                    state.blackName = state.blackName + " (AI)";
                }
                ResetGameState(board, state, pieceSelected, legalMovesForSelected);
                appState = AppState::Playing;
            }
            continue;
        }

        // ---- Playing state ----
        float dt = GetFrameTime();
        if (!state.gameOver) {
            if (board.turn == PieceColor::White) state.whiteTimeRemaining -= dt;
            else state.blackTimeRemaining -= dt;
            if (state.whiteTimeRemaining < 0) state.whiteTimeRemaining = 0;
            if (state.blackTimeRemaining < 0) state.blackTimeRemaining = 0;
        }

        bool isAiTurn = !state.gameOver && (mode == GameMode::HumanVsAi) && (board.turn == aiColor);
        if (isAiTurn) {
            bool aiHasMoves = HasAnyLegalMoves(board, aiColor);
            if (aiHasMoves) {
                Move aiMove = ChooseAiMove(board, aiColor, AI_RANDOM_MOVE_CHANCE, AI_SEARCH_DEPTH);
                PerformMoveWithLogging(board, state, aiMove);
            }
        }

        bool isHumanTurn = !isAiTurn && !state.gameOver;
        if (isHumanTurn && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int clickedRow, clickedCol;
            if (ScreenToBoard(GetMousePosition(), clickedRow, clickedCol)) {
                if (!pieceSelected) {
                    const Piece& clickedPiece = board.squares[clickedRow][clickedCol];
                    if (clickedPiece.type != PieceType::None && clickedPiece.color == board.turn) {
                        pieceSelected = true;
                        selectedRow = clickedRow;
                        selectedCol = clickedCol;
                        legalMovesForSelected = GenerateLegalMoves(board, selectedRow, selectedCol);
                    }
                }
                else {
                    if (IsLegalDestination(legalMovesForSelected, clickedRow, clickedCol)) {
                        Move chosenMove{ selectedRow, selectedCol, clickedRow, clickedCol };
                        PerformMoveWithLogging(board, state, chosenMove);
                        pieceSelected = false;
                        legalMovesForSelected.clear();
                    }
                    else {
                        const Piece& clickedPiece = board.squares[clickedRow][clickedCol];
                        if (clickedPiece.type != PieceType::None && clickedPiece.color == board.turn) {
                            selectedRow = clickedRow;
                            selectedCol = clickedCol;
                            legalMovesForSelected = GenerateLegalMoves(board, selectedRow, selectedCol);
                        }
                        else {
                            pieceSelected = false;
                            legalMovesForSelected.clear();
                        }
                    }
                }
            }
        }

        // ---- Drawing ----
        BeginDrawing();
        ClearBackground(Theme::SlateCharcoal);

        DrawBoardFrame();
        DrawTopBar(state, board);
        DrawBoardSquares();

        if (pieceSelected) {
            DrawSelectedSquareHighlight(selectedRow, selectedCol);
        }

        DrawPieces(board);

        if (pieceSelected) {
            DrawLegalMoveDots(legalMovesForSelected);
        }

        if (!state.gameOver && IsKingInCheck(board, board.turn)) {
            DrawCheckBanner(board.turn);
        }

        DrawBottomBar(state);

        bool menuClicked = DrawMainMenuButton();
        bool resetClicked = DrawNewGameButton();

        DrawResultBanner(state);

        EndDrawing();

        if (menuClicked) {
            appState = AppState::Menu;
        }
        if (resetClicked) {
            ResetGameState(board, state, pieceSelected, legalMovesForSelected);
        }
    }

    UnloadPieceTextures();
    CloseWindow();
    return 0;
}

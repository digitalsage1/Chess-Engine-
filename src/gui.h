#pragma once
#include "raylib.h"
#include "board.h"
#include "movegen.h"
#include "gamestate.h"
#include <vector>
#include <string>

// ============================================================================
// THEME / DESIGN TOKENS
// ============================================================================
// Palette: deep navy + copper accent + muted teal, replacing the earlier
// walnut/gold scheme. Navy reads as more "serious strategy game" than brown,
// copper gives a warm contrast point against the cool navy without drifting
// into generic blue+gold.

namespace Theme {
    inline Color NavyDeep = { 18, 28, 48, 255 };   // primary panel / frame background
    inline Color NavyMid = { 32, 47, 79, 255 };   // secondary panels, idle buttons
    inline Color NavyLight = { 46, 66, 104, 255 };  // hover state for navy elements
    inline Color Ivory = { 242, 236, 222, 255 }; // light squares
    inline Color SlateCharcoal = { 30, 33, 38, 255 };    // dark squares
    inline Color Copper = { 217, 142, 74, 255 };  // primary accent (active turn, gold-equivalent)
    inline Color CopperDim = { 217, 142, 74, 70 };   // soft accent fill (active-turn glow)
    inline Color CopperBright = { 232, 165, 99, 255 };  // hover state for copper elements
    inline Color Teal = { 95, 168, 160, 210 };  // legal move dots
    inline Color CoralRed = { 228, 97, 90, 255 };   // check warning
    inline Color TextLight = { 242, 236, 222, 255 }; // text on dark backgrounds
    inline Color TextDark = { 22, 26, 34, 255 };    // text on light/copper backgrounds
    inline Color Selected = { 217, 142, 74, 140 };  // selected-square highlight
    inline Color Overlay = { 10, 14, 24, 170 };    // dimmed background behind banners/modals
}

// ============================================================================
// LAYOUT
// ============================================================================
namespace Layout {
    inline int SQUARE_SIZE = 70;
    inline int BOARD_PIXELS = 8 * SQUARE_SIZE;
    inline int BORDER_THICK = 16;
    inline int TOP_BAR_H = 90;
    inline int BOTTOM_BAR_H = 150;

    inline int WindowWidth() { return BOARD_PIXELS + BORDER_THICK * 2; }
    inline int WindowHeight() { return TOP_BAR_H + BORDER_THICK * 2 + BOARD_PIXELS + BOTTOM_BAR_H; }
    inline int BoardOriginX() { return BORDER_THICK; }
    inline int BoardOriginY() { return TOP_BAR_H + BORDER_THICK; }
}

enum class GameMode { None, HumanVsHuman, HumanVsAi };

// ============================================================================
// REUSABLE WIDGETS
// ============================================================================

// Draws a button with a hover glow + slight upward lift effect. Returns true
// the frame it's clicked. Pass fontSize/primary for visual tuning.
bool DrawStyledButton(Rectangle rect, const char* label, int fontSize = 18, bool primary = false);

// Draws a simple text input box. Returns nothing - mutates `value` directly
// based on keyboard input when `isActive` is true. `isActive` is determined
// by the caller (typically: did the user click this box's rect this frame).
void DrawTextInputBox(Rectangle rect, const std::string& value, bool isActive, const char* placeholder);

// ============================================================================
// SCREENS
// ============================================================================

// Name entry screen. Returns true once the player confirms (button or Enter).
bool DrawNameEntryScreen(std::string& whiteNameInput, std::string& blackNameInput, int& activeField);

// Mode select screen. Returns the chosen mode, or GameMode::None if not yet chosen.
GameMode DrawModeMenu();

// ============================================================================
// IN-GAME UI
// ============================================================================

void DrawBoardFrame();
void DrawBoardSquares();
void DrawSelectedSquareHighlight(int row, int col);
void DrawLegalMoveDots(const std::vector<Move>& moves);
void DrawCheckBanner(PieceColor playerInCheck);
void DrawTopBar(const GameState& state, const Board& board);
void DrawBottomBar(const GameState& state);
void DrawResultBanner(const GameState& state);

// Returns true if "Main Menu" was clicked this frame.
bool DrawMainMenuButton();
// Returns true if "New Game" was clicked this frame.
bool DrawNewGameButton();

// ============================================================================
// PIECE TEXTURES
// ============================================================================
void LoadPieceTextures();
void UnloadPieceTextures();
void DrawPieces(const Board& board);

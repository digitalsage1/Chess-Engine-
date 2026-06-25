#include "gui.h"
#include <cstdio>

using namespace Theme;
using namespace Layout;

// ============================================================================
// REUSABLE WIDGETS
// ============================================================================

bool DrawStyledButton(Rectangle rect, const char* label, int fontSize, bool primary) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);

    // Subtle "lift" effect on hover: button shifts up 2px and gets a soft drop shadow
    Rectangle drawRect = rect;
    if (hover) drawRect.y -= 2;

    Color baseColor = primary ? Copper : NavyMid;
    Color hoverColor = primary ? CopperBright : NavyLight;
    Color textColor = primary ? TextDark : TextLight;

    // Drop shadow (drawn slightly offset, only visible when not hovering since hover "lifts" it)
    Rectangle shadowRect = { rect.x + 3, rect.y + 4, rect.width, rect.height };
    DrawRectangleRec(shadowRect, Color{ 0, 0, 0, 60 });

    DrawRectangleRec(drawRect, hover ? hoverColor : baseColor);
    DrawRectangleLinesEx(drawRect, hover ? 3 : 2, hover ? CopperBright : Copper);

    int textWidth = MeasureText(label, fontSize);
    DrawText(label, drawRect.x + (drawRect.width - textWidth) / 2,
        drawRect.y + (drawRect.height - fontSize) / 2, fontSize, textColor);

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void DrawTextInputBox(Rectangle rect, const std::string& value, bool isActive, const char* placeholder) {
    Color borderColor = isActive ? Copper : NavyLight;
    Color fillColor = isActive ? NavyLight : NavyMid;

    DrawRectangleRec(rect, fillColor);
    DrawRectangleLinesEx(rect, isActive ? 3 : 2, borderColor);

    // Thin accent underline for extra polish when active
    if (isActive) {
        DrawRectangle(rect.x, rect.y + rect.height - 3, rect.width, 3, Copper);
    }

    const char* displayText = value.empty() ? placeholder : value.c_str();
    Color textColor = value.empty() ? Color{ 150, 150, 160, 255 } : TextLight;
    DrawText(displayText, rect.x + 16, rect.y + (rect.height - 24) / 2, 24, textColor);

    // Blinking text cursor when active
    if (isActive && ((int)(GetTime() * 2) % 2 == 0)) {
        int textWidth = MeasureText(value.c_str(), 24);
        DrawRectangle(rect.x + 16 + textWidth + 3, rect.y + (rect.height - 26) / 2, 2, 26, Copper);
    }
}

// ============================================================================
// SCREENS
// ============================================================================

bool DrawNameEntryScreen(std::string& whiteNameInput, std::string& blackNameInput, int& activeField) {
    int W = WindowWidth();

    ClearBackground(SlateCharcoal);

    // Soft vertical gradient feel: a darker navy band behind the title area
    DrawRectangle(0, 0, W, 300, NavyDeep);
    DrawRectangle(0, 296, W, 4, Copper);

    const char* title = "Chess Engine";
    int titleSize = 48;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (W - titleWidth) / 2, 70, titleSize, Copper);

    const char* subtitle = "Enter player names to begin";
    int subSize = 20;
    int subWidth = MeasureText(subtitle, subSize);
    DrawText(subtitle, (W - subWidth) / 2, 135, subSize, TextLight);

    Rectangle field1 = { (float)(W / 2 - 170), 320, 340, 56 };
    Rectangle field2 = { (float)(W / 2 - 170), 400, 340, 56 };

    int labelSize = 18;
    DrawText("WHITE", field1.x, field1.y - 21, labelSize, Copper);
    DrawText("BLACK / AI", field2.x, field2.y - 21, labelSize, Copper);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, field1)) activeField = 1;
        else if (CheckCollisionPointRec(mouse, field2)) activeField = 2;
    }

    std::string* activeString = (activeField == 1) ? &whiteNameInput : (activeField == 2) ? &blackNameInput : nullptr;
    if (activeString) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 125 && activeString->size() < 16) {
                activeString->push_back((char)key);
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !activeString->empty()) {
            activeString->pop_back();
        }
    }

    DrawTextInputBox(field1, whiteNameInput, activeField == 1, "Player 1");
    DrawTextInputBox(field2, blackNameInput, activeField == 2, "Player 2");

    Rectangle continueButton = { (float)(W / 2 - 100), 490, 200, 54 };
    bool clicked = DrawStyledButton(continueButton, "Continue", 22, true);

    return clicked || IsKeyPressed(KEY_ENTER);
}

GameMode DrawModeMenu() {
    int W = WindowWidth();
    int H = WindowHeight();

    ClearBackground(SlateCharcoal);
    DrawRectangle(0, 0, W, H, NavyDeep);

    const char* title = "Choose Game Mode";
    int titleWidth = MeasureText(title, 30);
    DrawText(title, (W - titleWidth) / 2, 160, 30, Copper);

    Rectangle hvhButton = { (float)(W / 2 - 140), 280, 280, 56 };
    Rectangle hvaiButton = { (float)(W / 2 - 140), 360, 280, 56 };

    bool hvhClicked = DrawStyledButton(hvhButton, "Human vs Human", 19);
    bool hvaiClicked = DrawStyledButton(hvaiButton, "Human vs AI", 19);

    if (hvhClicked) return GameMode::HumanVsHuman;
    if (hvaiClicked) return GameMode::HumanVsAi;
    return GameMode::None;
}

// ============================================================================
// IN-GAME UI
// ============================================================================

void DrawBoardFrame() {
    int W = WindowWidth();
    Rectangle outer = { 0, (float)TOP_BAR_H, (float)W, (float)(BOARD_PIXELS + BORDER_THICK * 2) };
    DrawRectangleRec(outer, NavyDeep);

    int ox = BoardOriginX(), oy = BoardOriginY();
    // Double-line frame for a bit more visual richness than a single border
    DrawRectangleLinesEx({ (float)(ox - 6), (float)(oy - 6), (float)(BOARD_PIXELS + 12), (float)(BOARD_PIXELS + 12) }, 1, NavyLight);
    DrawRectangleLinesEx({ (float)(ox - 3), (float)(oy - 3), (float)(BOARD_PIXELS + 6), (float)(BOARD_PIXELS + 6) }, 2, Copper);
}

void DrawBoardSquares() {
    int ox = BoardOriginX(), oy = BoardOriginY();
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Color squareColor = ((row + col) % 2 == 0) ? Ivory : SlateCharcoal;
            DrawRectangle(ox + col * SQUARE_SIZE, oy + row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, squareColor);
        }
    }
}

void DrawSelectedSquareHighlight(int row, int col) {
    int ox = BoardOriginX(), oy = BoardOriginY();
    DrawRectangle(ox + col * SQUARE_SIZE, oy + row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, Selected);
    DrawRectangleLinesEx({ (float)(ox + col * SQUARE_SIZE), (float)(oy + row * SQUARE_SIZE), (float)SQUARE_SIZE, (float)SQUARE_SIZE }, 3, Copper);
}

void DrawLegalMoveDots(const std::vector<Move>& moves) {
    int ox = BoardOriginX(), oy = BoardOriginY();
    for (const Move& m : moves) {
        int cx = ox + m.toCol * SQUARE_SIZE + SQUARE_SIZE / 2;
        int cy = oy + m.toRow * SQUARE_SIZE + SQUARE_SIZE / 2;
        DrawCircle(cx, cy, 10, Teal);
        DrawCircleLines(cx, cy, 10, Color{ 255, 255, 255, 90 });
    }
}

void DrawCheckBanner(PieceColor playerInCheck) {
    int W = WindowWidth();
    int oy = BoardOriginY();
    const char* checkText = (playerInCheck == PieceColor::White) ? "White is in check" : "Black is in check";
    int checkWidth = MeasureText(checkText, 18);
    Rectangle banner = { (float)(W / 2 - checkWidth / 2 - 14), (float)(oy + 8), (float)(checkWidth + 28), 30 };
    DrawRectangleRec(banner, Color{ 0, 0, 0, 150 });
    DrawRectangleLinesEx(banner, 2, CoralRed);
    DrawText(checkText, banner.x + 14, banner.y + 6, 18, CoralRed);
}

void DrawTopBar(const GameState& state, const Board& board) {
    int W = WindowWidth();
    DrawRectangle(0, 0, W, TOP_BAR_H, NavyDeep);
    DrawRectangle(0, TOP_BAR_H - 3, W, 3, Copper);

    bool whiteActive = (board.turn == PieceColor::White) && !state.gameOver;
    bool blackActive = (board.turn == PieceColor::Black) && !state.gameOver;

    Rectangle whitePanel = { 14, 12, (float)(W / 2 - 20), (float)(TOP_BAR_H - 24) };
    if (whiteActive) {
        DrawRectangleRec(whitePanel, CopperDim);
        DrawRectangleLinesEx(whitePanel, 2, Copper);
    }
    else {
        DrawRectangleLinesEx(whitePanel, 1, NavyLight);
    }
    DrawText(state.whiteName.c_str(), whitePanel.x + 10, whitePanel.y + 6, 18, TextLight);
    std::string whiteClock = FormatClockTime(state.whiteTimeRemaining);
    DrawText(whiteClock.c_str(), whitePanel.x + 10, whitePanel.y + 30, 24, whiteActive ? Copper : TextLight);

    Rectangle blackPanel = { (float)(W / 2 + 6), 12, (float)(W / 2 - 20), (float)(TOP_BAR_H - 24) };
    if (blackActive) {
        DrawRectangleRec(blackPanel, CopperDim);
        DrawRectangleLinesEx(blackPanel, 2, Copper);
    }
    else {
        DrawRectangleLinesEx(blackPanel, 1, NavyLight);
    }
    int blackNameWidth = MeasureText(state.blackName.c_str(), 18);
    DrawText(state.blackName.c_str(), blackPanel.x + blackPanel.width - blackNameWidth - 10, blackPanel.y + 6, 18, TextLight);
    std::string blackClock = FormatClockTime(state.blackTimeRemaining);
    int blackClockWidth = MeasureText(blackClock.c_str(), 24);
    DrawText(blackClock.c_str(), blackPanel.x + blackPanel.width - blackClockWidth - 10, blackPanel.y + 30, 24, blackActive ? Copper : TextLight);
}

void DrawBottomBar(const GameState& state) {
    int W = WindowWidth();
    int barY = BoardOriginY() + BOARD_PIXELS + BORDER_THICK;
    DrawRectangle(0, barY, W, BOTTOM_BAR_H, NavyDeep);
    DrawRectangle(0, barY, W, 3, Copper);

    DrawText("MOVE LOG", 16, barY + 10, 14, Copper);

    int lineHeight = 20;
    int maxLines = (BOTTOM_BAR_H - 40) / lineHeight;
    int startIndex = (int)state.moveLog.size() - 1;

    int col1X = 16, col2X = W / 2 + 8;
    int y = barY + 32;
    int lineCount = 0;

    for (int i = startIndex; i >= 0 && lineCount < maxLines * 2; i--) {
        const MoveLogEntry& entry = state.moveLog[i];
        int x = (lineCount % 2 == 0) ? col1X : col2X;
        Color textColor = (entry.playerColor == PieceColor::White) ? TextLight : Copper;

        std::string prefix = (entry.playerColor == PieceColor::White) ? "W  " : "B  ";
        DrawText((prefix + entry.text).c_str(), x, y, 14, textColor);

        if (lineCount % 2 == 1) y += lineHeight;
        lineCount++;
    }
}

void DrawResultBanner(const GameState& state) {
    if (!state.gameOver) return;

    int W = WindowWidth(), H = WindowHeight();
    DrawRectangle(0, 0, W, H, Overlay);

    Rectangle banner = { (float)(W / 2 - 220), (float)(H / 2 - 75), 440, 150 };
    DrawRectangleRec({ banner.x + 4, banner.y + 6, banner.width, banner.height }, Color{ 0, 0, 0, 80 });
    DrawRectangleRec(banner, NavyDeep);
    DrawRectangleLinesEx(banner, 1, NavyLight);
    DrawRectangleLinesEx({ banner.x + 3, banner.y + 3, banner.width - 6, banner.height - 6 }, 2, Copper);

    int fontSize = 24;
    int textWidth = MeasureText(state.resultText.c_str(), fontSize);
    DrawText(state.resultText.c_str(), banner.x + (banner.width - textWidth) / 2, banner.y + 40, fontSize, Copper);

    const char* subtext = "Click 'New Game' below to play again";
    int subWidth = MeasureText(subtext, 14);
    DrawText(subtext, banner.x + (banner.width - subWidth) / 2, banner.y + 85, 14, TextLight);
}

bool DrawMainMenuButton() {
    int W = WindowWidth();
    Rectangle rect = { (float)(W - 2 * 120 - 10 - 12), (float)(BoardOriginY() + BOARD_PIXELS + BORDER_THICK + 10), 110, 32 };
    return DrawStyledButton(rect, "Main Menu", 14);
}

bool DrawNewGameButton() {
    int W = WindowWidth();
    Rectangle rect = { (float)(W - 120 - 12), (float)(BoardOriginY() + BOARD_PIXELS + BORDER_THICK + 10), 110, 32 };
    return DrawStyledButton(rect, "New Game", 14, true);
}

// ============================================================================
// PIECE TEXTURES
// ============================================================================

static Texture2D pieceTextures[7][3];

void LoadPieceTextures() {
    struct TypeName { PieceType type; const char* name; };
    TypeName typeNames[] = {
        { PieceType::Pawn,   "pawn"   }, { PieceType::Knight, "knight" },
        { PieceType::Bishop, "bishop" }, { PieceType::Rook,   "rook"   },
        { PieceType::Queen,  "queen"  }, { PieceType::King,   "king"   },
    };
    struct ColorName { PieceColor color; const char* name; };
    ColorName colorNames[] = { { PieceColor::White, "white" }, { PieceColor::Black, "black" } };

    for (auto& t : typeNames) {
        for (auto& c : colorNames) {
            const char* path = TextFormat("assets/%s_%s.png", c.name, t.name);
            Texture2D tex = LoadTexture(path);
            pieceTextures[(int)t.type][(int)c.color] = tex;
            if (tex.id == 0) printf("WARNING: failed to load texture: %s\n", path);
        }
    }
}

void UnloadPieceTextures() {
    for (int t = 0; t < 7; t++)
        for (int c = 0; c < 3; c++)
            if (pieceTextures[t][c].id != 0) UnloadTexture(pieceTextures[t][c]);
}

void DrawPieces(const Board& board) {
    int ox = BoardOriginX(), oy = BoardOriginY();
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            const Piece& piece = board.squares[row][col];
            if (piece.type == PieceType::None) continue;

            Texture2D tex = pieceTextures[(int)piece.type][(int)piece.color];
            if (tex.id == 0) continue;

            float margin = SQUARE_SIZE * 0.05f;
            float maxDim = SQUARE_SIZE - margin * 2;
            float scale = maxDim / (float)((tex.width > tex.height) ? tex.width : tex.height);
            float drawWidth = tex.width * scale;
            float drawHeight = tex.height * scale;

            float x = ox + col * SQUARE_SIZE + (SQUARE_SIZE - drawWidth) / 2.0f;
            float y = oy + row * SQUARE_SIZE + (SQUARE_SIZE - drawHeight) / 2.0f;

            DrawTextureEx(tex, { x, y }, 0.0f, scale, WHITE);
        }
    }
}


#include "raylib.h"

int main() {
    InitWindow(800, 800, "Chess Engine");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Raylib is working!", 250, 380, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
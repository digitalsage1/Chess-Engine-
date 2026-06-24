# ♟️ RayChess Engine

A high-performance, hardware-accelerated Graphical Chess Engine built from scratch using **C++** and **Raylib**. Featuring a fully interactive, responsive user interface alongside a custom-designed, multi-threaded Artificial Intelligence decision engine. 

No sluggish web views, no bloated electron frameworks, no heavy engines. Just pure, close-to-the-metal simulation driven by deterministic logic and OpenGL acceleration.

---

## 🚀 Key Features

*   **GPU-Accelerated GUI:** Silky smooth 60 FPS rendering courtesy of Raylib. Features sub-millisecond input handling, smooth piece dragging, fluid hover animations, and zero-flicker board state updates.
*   **Custom Chess AI Engine:** Integrated chess brain utilizing deep search algorithms. The AI evaluates thousands of positions per second to deliver an scalable, challenging single-player experience.
*   **Dual Gameplay Modes:** 
    *   `Human vs Human (Local)`: Pass-and-play configuration with real-time turn tracking and automated board flipping (optional).
    *   `Human vs AI`: Test your strategic limits against a deterministic silicon adversary with adjustable search depth.
*   **Rigorous Move Validation:** Flawless implementation of complete FIDE rules, including complex edge cases:
    *   *En Passant* detection and immediate expiration.
    *   Two-square pawn initial advances and pinning states.
    *   King/Rook castling rights tracking (broken if pieces move or pass through check).
    *   Pawn promotion mechanics with real-time UI selection overlay.
    *   Endgame states: Checkmate, Stalemate (no legal moves), and the Threefold Repetition baseline.
*   **Clean Architectural Abstraction:** Implements a strict model-view-controller adjacent separation between Core Bitboards/Arrays, the AI Search Pipeline, and the Raylib Render Loop.

---

## 🛠️ System Architecture

The engine is modularized into three distinct foundational subsystems to ensure high maintainability and clean optimization paths:

┌──────────────────────────────┐
              │      Raylib GUI Layer        │
              │  (Input, Textures, Rendering)│
              └──────────────┬───────────────┘
                             │  Sends Inputs / Polling
                             ▼
              ┌──────────────────────────────┐
              │     Core Game State Engine   │
              │  (Move Validation, Rulesets) │
              └──────────────┬───────────────┘
                             │  Provides Legal Moves
                             ▼
              ┌──────────────────────────────┐
              │      AI Decision Engine      │
              │ (Minimax, Pruning, Evaluation)│
              └──────────────────────────────┘

              ### 1. The Core Board Representation
*   Manages the 8x8 matrix representation (or Bitboard structure) of the active playing grid.
*   Maintains atomic state variables: `active_player`, `castling_rights`, `en_passant_target_square`, and `halfmove_clock`.
*   Generates a precise vector of all mathematically legal moves for a given state before any input processing.

### 2. The Graphical Interface (Raylib)
*   Translates internal grid coordinates `(0-7, 0-7)` into explicit screen coordinates based on dynamic window dimensions.
*   Listens to mouse input vectors for precise drag-and-drop mechanics or click-to-move sequences.
*   Renders customized sprite textures using optimized texture batching.

### 3. The AI Decision Pipeline
*   Invoked asynchronously or synchronously on the player's alternate turn.
*   Evaluates the board state scoring heuristic dynamically from the perspective of the maximizing agent.



## 🧠 The AI Brain: Under the Hood

___

The AI isn't just checking random branches; it processes the game tree using an advanced alpha-beta pruning optimization pipeline:
___

## 📦 Installation & Build Pipeline

### Prerequisites
*   A modern C++ compiler supporting at least **C++17** standard (GCC, Clang, or MSVC).
*   **Raylib Development Libraries** installed locally on your system.

***

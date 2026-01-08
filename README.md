# CLIg - Command Line Games

A collection of simple terminal games written in C using ncurses.

## Projects

### 1. Tetris (`/tetris`)
A Tetris clone featuring:
-   **Proper Mechanics**: 7-Bag Randomizer, SRS Wall Kicks, Move Reset.
-   **Controls**: WASD/Arrow Keys, Space (Hard Drop), S (Fast Soft Drop), C (Hold).
-   **Physics**: Tuned DAS (90ms) and ARR (45ms).

### 2. Snake (`/snake`)
A classic Snake implementation with:
-   **Controls**: WASD / Arrow Keys.
-   **Features**: Smooth input queueing, score tracking.

### 3. 2048 (`/2048`)
A classic sliding tile puzzle game:
-   **Controls**: WASD / Arrow Keys.
-   **Features**: Score tracking, tile merging, 2048 win condition.

## Build & Run

Each project has its own Makefile.

```bash
# Run Tetris
cd tetris
make
./tetris

# Run Snake
cd ../snake
make
./snake
```

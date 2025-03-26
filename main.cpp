#include <iostream>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <algorithm>

using namespace std;

// Console manipulation functions (Windows-specific)
void SetConsoleCursorPositionXY(int x, int y) {
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void ShowConsoleCursor(bool showFlag) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag;
    SetConsoleCursorInfo(out, &cursorInfo);
}

// Constants
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int BLOCK_SIZE = 2; // For visual representation (2x2 block in console)
const string HIGH_SCORE_FILE = "highscore.txt";

// Tetromino shapes (I, O, T, S, Z, J, L)
const vector<vector<vector<int>>> SHAPES = {
    {{1, 1, 1, 1}}, // I
    {{1, 1}, {1, 1}}, // O
    {{0, 1, 0}, {1, 1, 1}}, // T
    {{0, 1, 1}, {1, 1, 0}}, // S
    {{1, 1, 0}, {0, 1, 1}}, // Z
    {{1, 0, 0}, {1, 1, 1}}, // J
    {{0, 0, 1}, {1, 1, 1}}  // L
};

// Colors for different tetrominoes
const int COLORS[] = { 9, 14, 13, 10, 12, 11, 15 }; // Blue, Yellow, Magenta, Green, Red, Cyan, White

class Tetromino {
public:
    vector<vector<int>> shape;
    int x, y; // Position on the board
    int color;

    Tetromino(int type) {
        shape = SHAPES[type];
        color = COLORS[type];
        x = BOARD_WIDTH / 2 - shape[0].size() / 2;
        y = 0; // Start at the top
    }

    void rotate() {
        int rows = shape.size();
        int cols = shape[0].size();
        vector<vector<int>> rotated(cols, vector<int>(rows));
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                rotated[j][rows - 1 - i] = shape[i][j];
            }
        }
        shape = rotated;
    }
};

class Board {
public:
    vector<vector<int>> grid; // 0 = empty, >0 = occupied (color value)
    Tetromino* currentTetromino;

    Board() : grid(BOARD_HEIGHT, vector<int>(BOARD_WIDTH, 0)), currentTetromino(nullptr) {}

    ~Board() {
        if (currentTetromino) {
            delete currentTetromino;
            currentTetromino = nullptr;
        }
    }

    bool isCollision(int newX, int newY, const vector<vector<int>>& shape) const {
        for (int i = 0; i < shape.size(); i++) {
            for (int j = 0; j < shape[i].size(); j++) {
                if (shape[i][j]) {
                    int boardX = newX + j;
                    int boardY = newY + i;
                    if (boardX < 0 || boardX >= BOARD_WIDTH || boardY >= BOARD_HEIGHT ||
                        (boardY >= 0 && grid[boardY][boardX] > 0)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void mergeTetromino() {
        if (!currentTetromino) return;
        for (int i = 0; i < currentTetromino->shape.size(); i++) {
            for (int j = 0; j < currentTetromino->shape[i].size(); j++) {
                if (currentTetromino->shape[i][j]) {
                    int boardX = currentTetromino->x + j;
                    int boardY = currentTetromino->y + i;
                    if (boardY >= 0) {
                        grid[boardY][boardX] = currentTetromino->color;
                    }
                }
            }
        }
    }

    int clearLines() {
        int linesCleared = 0;
        for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
            bool full = true;
            for (int j = 0; j < BOARD_WIDTH; j++) {
                if (grid[i][j] == 0) {
                    full = false;
                    break;
                }
            }
            if (full) {
                linesCleared++;
                grid.erase(grid.begin() + i);
                grid.insert(grid.begin(), vector<int>(BOARD_WIDTH, 0));
            }
        }
        return linesCleared;
    }

    bool isGameOver() const {
        if (!currentTetromino) return false;
        return isCollision(currentTetromino->x, currentTetromino->y, currentTetromino->shape);
    }
};

class Game {
private:
    Board board;
    long long score;
    long long highScore;
    bool gameOver;
    bool paused;
    int fallDelayMs;
    const int BASE_FALL_DELAY_MS = 500;
    Tetromino* nextTetromino;
    vector<vector<int>> lastDisplayGrid;
    bool firstMove;

    void loadHighScore() {
        ifstream file(HIGH_SCORE_FILE);
        if (file.is_open()) {
            file >> highScore;
            file.close();
        } else {
            highScore = 0;
            cerr << "Warning: Could not load high score file." << endl;
        }
    }
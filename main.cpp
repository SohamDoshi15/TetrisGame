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

    void saveHighScore() {
        if (score > highScore) {
            highScore = score;
            ofstream file(HIGH_SCORE_FILE); // Fixed typo here
            if (file.is_open()) {
                file << highScore;
                file.close();
            } else {
                cerr << "Warning: Could not save high score to file." << endl;
            }
        }
    }

    void spawnTetromino() {
        if (board.currentTetromino) {
            delete board.currentTetromino;
            board.currentTetromino = nullptr;
        }
        if (nextTetromino) {
            board.currentTetromino = nextTetromino;
            nextTetromino = nullptr;
        } else {
            int type = rand() % SHAPES.size();
            board.currentTetromino = new Tetromino(type);
        }
        firstMove = true;

        if (board.isGameOver()) {
            gameOver = true;
            cerr << "Game Over: New tetromino cannot spawn at (" << board.currentTetromino->x << ", " << board.currentTetromino->y << ")" << endl;
            return;
        }

        int nextType = rand() % SHAPES.size();
        nextTetromino = new Tetromino(nextType);
    }

    void handleInput() {
        if (_kbhit()) {
            char key = _getch();
            if (key == 27) { // ESC to quit
                gameOver = true;
                return;
            }
            if (key == 'p' || key == 'P') {
                paused = !paused;
                if (paused) {
                    SetConsoleCursorPositionXY(0, BOARD_HEIGHT + 3);
                    cout << "PAUSED - Press P to resume";
                } else {
                    render();
                }
                return;
            }
            if (paused) return;

            int newX = board.currentTetromino->x;
            int newY = board.currentTetromino->y;

            switch (key) {
            case 'a': // Move left
                newX--;
                break;
            case 'd': // Move right
                newX++;
                break;
            case 's': // Move down
                newY++;
                break;
            case 'w': // Rotate
                {
                    vector<vector<int>> tempShape = board.currentTetromino->shape;
                    board.currentTetromino->rotate();
                    if (board.isCollision(newX, newY, board.currentTetromino->shape)) {
                        board.currentTetromino->shape = tempShape;
                    }
                }
                break;
            }

            if (!board.isCollision(newX, newY, board.currentTetromino->shape)) {
                board.currentTetromino->x = newX;
                board.currentTetromino->y = newY;
                firstMove = false;
            }
        }
    }
    void update() {
        if (paused) return;

        if (!board.currentTetromino) {
            spawnTetromino();
            return;
        }

        int newY = board.currentTetromino->y + 1;
        if (!board.isCollision(board.currentTetromino->x, newY, board.currentTetromino->shape)) {
            board.currentTetromino->y = newY;
            firstMove = false;
        } else {
            board.mergeTetromino();
            score += 10; // Score for piece landing
            int linesCleared = board.clearLines();
            if (linesCleared > 0) {
                score += linesCleared * 100; // Additional score for cleared lines
            }
            delete board.currentTetromino;
            board.currentTetromino = nullptr;
            fallDelayMs = std::max(100, BASE_FALL_DELAY_MS - static_cast<int>(score / 500) * 50);
        }

        if (!firstMove && board.isGameOver()) {
            gameOver = true;
        }
    }

    void render() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        static int lastScore = -1;
        static int lastHighScore = -1;
        if (score != lastScore || highScore != lastHighScore) {
            SetConsoleCursorPositionXY(0, 0);
            cout << "Score: " << score << "  High Score: " << highScore;
            lastScore = score;
            lastHighScore = highScore;
        }

        vector<vector<int>> displayGrid = board.grid;
        if (board.currentTetromino) {
            for (int i = 0; i < board.currentTetromino->shape.size(); i++) {
                for (int j = 0; j < board.currentTetromino->shape[i].size(); j++) {
                    if (board.currentTetromino->shape[i][j]) {
                        int boardX = board.currentTetromino->x + j;
                        int boardY = board.currentTetromino->y + i;
                        if (boardY >= 0 && boardX >= 0 && boardX < BOARD_WIDTH && boardY < BOARD_HEIGHT) {
                            displayGrid[boardY][boardX] = board.currentTetromino->color;
                        }
                    }
                }
            }
        }

        if (lastDisplayGrid.size() != BOARD_HEIGHT || 
            (lastDisplayGrid.size() > 0 && lastDisplayGrid[0].size() != BOARD_WIDTH)) {
            lastDisplayGrid = vector<vector<int>>(BOARD_HEIGHT, vector<int>(BOARD_WIDTH, -1));
        }
        for (int i = 0; i < BOARD_HEIGHT; i++) {
            for (int j = 0; j < BOARD_WIDTH; j++) {
                if (displayGrid[i][j] != lastDisplayGrid[i][j]) {
                    SetConsoleCursorPositionXY(j * BLOCK_SIZE + 1, i + 1);
                    if (displayGrid[i][j] > 0) {
                        SetConsoleTextAttribute(hConsole, displayGrid[i][j]);
                        cout << "[]";
                    } else {
                        SetConsoleTextAttribute(hConsole, 7);
                        cout << "  ";
                    }
                    SetConsoleTextAttribute(hConsole, 7); // Reset color after each block
                }
            }
        }
        lastDisplayGrid = displayGrid;

        static bool bordersDrawn = false;
        if (!bordersDrawn) {
            SetConsoleTextAttribute(hConsole, 7);
            for (int i = 0; i < BOARD_HEIGHT; i++) {
                SetConsoleCursorPositionXY(0, i + 1);
                cout << "|";
                SetConsoleCursorPositionXY(BOARD_WIDTH * BLOCK_SIZE + 1, i + 1);
                cout << "|";
            }
            SetConsoleCursorPositionXY(0, BOARD_HEIGHT + 1);
            cout << "+";
            for (int j = 0; j < BOARD_WIDTH * BLOCK_SIZE; j++) cout << "-";
            cout << "+";
            bordersDrawn = true;
        }

        static int lastNextColor = -1;
        static vector<vector<int>> lastNextShape;
        if (nextTetromino && (nextTetromino->color != lastNextColor || nextTetromino->shape != lastNextShape)) {
            SetConsoleCursorPositionXY(BOARD_WIDTH * BLOCK_SIZE + 4, 2);
            cout << "Next:";
            for (int i = 0; i < 4; i++) {
                SetConsoleCursorPositionXY(BOARD_WIDTH * BLOCK_SIZE + 4, 3 + i);
                cout << "        ";
            }
            for (int i = 0; i < nextTetromino->shape.size(); i++) {
                SetConsoleCursorPositionXY(BOARD_WIDTH * BLOCK_SIZE + 4, 3 + i);
                for (int j = 0; j < nextTetromino->shape[i].size(); j++) {
                    if (nextTetromino->shape[i][j]) {
                        SetConsoleTextAttribute(hConsole, nextTetromino->color);
                        cout << "[]";
                    } else {
                        cout << "  ";
                    }
                }
            }
            lastNextColor = nextTetromino->color;
            lastNextShape = nextTetromino->shape;
        }

        static bool instructionsDrawn = false;
        if (!instructionsDrawn) {
            SetConsoleCursorPositionXY(0, BOARD_HEIGHT + 2);
            cout << "Controls: A (Left), D (Right), S (Down), W (Rotate), P (Pause), ESC (Quit)";
            instructionsDrawn = true;
        }

        SetConsoleTextAttribute(hConsole, 7);
    }

public:
    Game() : score(0), highScore(0), gameOver(false), paused(false), fallDelayMs(BASE_FALL_DELAY_MS), nextTetromino(nullptr), firstMove(true) {
        srand(static_cast<unsigned>(time(nullptr)));
        ShowConsoleCursor(false);
        loadHighScore();
        system("cls");
        spawnTetromino();
        render();
    }

    ~Game() {
        if (nextTetromino) {
            delete nextTetromino;
        }
        saveHighScore();
        ShowConsoleCursor(true);
    }

    void run() {
        DWORD lastUpdate = GetTickCount();
        while (!gameOver) {
            handleInput();
            if (!paused) {
                DWORD currentTime = GetTickCount();
                if (currentTime - lastUpdate >= fallDelayMs) {
                    update();
                    lastUpdate = currentTime;
                }
            }
            render();
            Sleep(16);
        }
        system("cls");
        cout << "Game Over! Final Score: " << score << endl;
        cout << "High Score: " << highScore << endl;
    }
};

int main() {
    try {
        Game game;
        game.run();
    } catch (const exception& e) {
        cerr << "An error occurred: " << e.what() << endl;
        return 1;
    }
    return 0;
}


#include <iostream>
#include <chrono>
#include <thread>
#include <conio.h>
#include <windows.h>
#include <vector>

using namespace std;

const int WIDTH = 80;
const int HEIGHT = 25;
const int PADDLE_SPEED = 1;

int ballX, ballY;
int ballSpeedX, ballSpeedY;
int leftPaddleY, rightPaddleY;
int leftScore, rightScore;
int paddleSize;
int gameSpeed;
bool gameRunning;

vector<pair<int, int>> prevBallPositions;
vector<pair<int, int>> prevLeftPaddlePositions;
vector<pair<int, int>> prevRightPaddlePositions;

enum Difficulty { EASY = 1, MEDIUM, HARD };
Difficulty currentDifficulty;

// Очистка экрана перед началом игры
void ClearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    GetConsoleScreenBufferInfo(hConsole, &csbi);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coordScreen, &cCharsWritten);
    SetConsoleCursorPosition(hConsole, coordScreen);
}

// Очистка старых позиций мяча и ракеток
void ClearPosition(int x, int y) {
    COORD cursorPos;
    cursorPos.X = x;
    cursorPos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
    cout << " ";
}

// Отрисовка верхней и нижней границ игрового поля 
void DrawBorders() {
    COORD cursorPos;
    
    // Верхняя граница
    for (int x = 0; x < WIDTH; x++) {
        cursorPos.X = x;
        cursorPos.Y = 1;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
        cout << "-";
    }
    
    // Нижняя граница
    for (int x = 0; x < WIDTH; x++) {
        cursorPos.X = x;
        cursorPos.Y = HEIGHT - 1;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
        cout << "-";
    }
}

// Восстановление верхней границы окна после столкновения с мячом
void RedrawBorder(int x, int y) {
    COORD cursorPos;
    cursorPos.X = x;
    cursorPos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
    cout << "-";
}

// Инициализация игрового окна 
void SetupGame() {
    ClearScreen();
    
    ballX = WIDTH / 2;
    ballY = HEIGHT / 2;
    ballSpeedX = 1;
    ballSpeedY = 1;
    leftPaddleY = HEIGHT / 2;
    rightPaddleY = HEIGHT / 2;
    leftScore = 0;
    rightScore = 0;
    gameRunning = true;
    
    prevBallPositions.clear();
    prevLeftPaddlePositions.clear();
    prevRightPaddlePositions.clear();
    
    DrawBorders();
}

// Отрисовка динамических объектов игры
void Draw() {
    // Очистка предыдущих позиций
    for (auto& pos : prevBallPositions) {
        // Восстановление верхней границы
        if (pos.second == 1 || pos.second == HEIGHT - 1) {
            RedrawBorder(pos.first, pos.second);
        } else {
            ClearPosition(pos.first, pos.second);
        }
    }
    
    for (auto& pos : prevLeftPaddlePositions) {
        ClearPosition(pos.first, pos.second);
    }
    for (auto& pos : prevRightPaddlePositions) {
        ClearPosition(pos.first, pos.second);
    }
    
    // Хранение текущих позиций мяча и ракеток
    prevBallPositions.clear();
    prevBallPositions.push_back({ballX, ballY});
    
    prevLeftPaddlePositions.clear();
    for (int i = 0; i < paddleSize; i++) {
        prevLeftPaddlePositions.push_back({0, leftPaddleY + i});
    }
    
    prevRightPaddlePositions.clear();
    for (int i = 0; i < paddleSize; i++) {
        prevRightPaddlePositions.push_back({WIDTH - 1, rightPaddleY + i});
    }

    // Отрисовка мяча
    COORD cursorPos;
    cursorPos.X = ballX;
    cursorPos.Y = ballY;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
    cout << "O";

    // Отрисовка ракеток
    for (int i = 0; i < paddleSize; i++) {
        int yPos = leftPaddleY + i;
        if (yPos > 1 && yPos < HEIGHT - 1) {
            cursorPos.X = 0;
            cursorPos.Y = yPos;
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
            cout << "|";
        }
    }

    for (int i = 0; i < paddleSize; i++) {
        int yPos = rightPaddleY + i;
        if (yPos > 1 && yPos < HEIGHT - 1) {
            cursorPos.X = WIDTH - 1;
            cursorPos.Y = yPos;
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
            cout << "|";
        }
    }

    // Отображение счёта
    cursorPos.X = WIDTH / 2 - 5;
    cursorPos.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
    cout << leftScore << " - " << rightScore;

    // Скрытие курсора
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// Обработка ввода для управления ракетками
void Input() {
    if (_kbhit()) {
        int key = _getch();
        
        // Игрок 1 (левая ракетка)
        if (key == 'w' && leftPaddleY > 1) {
            leftPaddleY -= PADDLE_SPEED;
        }
        if (key == 's' && leftPaddleY < HEIGHT - paddleSize - 1) {
            leftPaddleY += PADDLE_SPEED;
        }
        
        // Игрок 2 (правая ракетка)
        if (key == 224) { // Arrow key prefix
            key = _getch(); // Get actual arrow key
            if (key == 72 && rightPaddleY > 1) { // Up arrow
                rightPaddleY -= PADDLE_SPEED;
            }
            if (key == 80 && rightPaddleY < HEIGHT - paddleSize - 1) { // Down arrow
                rightPaddleY += PADDLE_SPEED;
            }
        }
        
        if (key == 'q') {
            gameRunning = false;
        }
    }
}

// Логика игры
void Logic() {
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    // Отскок мяча от верхней и нижней границ
    if (ballY <= 1 || ballY >= HEIGHT - 2) {
        ballSpeedY = -ballSpeedY;
        // Для предотвращения застревания мяча в границах
        if (ballY <= 1) ballY = 2;
        if (ballY >= HEIGHT - 2) ballY = HEIGHT - 3;
    }

    if (ballX <= 1) {
        bool hit = false;
        for (int i = 0; i < paddleSize; i++) {
            if (ballY == leftPaddleY + i) {
                hit = true;
                break;
            }
        }
        if (hit) {
            ballSpeedX = -ballSpeedX;
            if (ballSpeedY == 0) ballSpeedY = (rand() % 2) ? 1 : -1;
        } else if (ballX <= 0) {
            rightScore++;
            ballX = WIDTH / 2;
            ballY = HEIGHT / 2;
            ballSpeedX = -ballSpeedX;
        }
    }

    if (ballX >= WIDTH - 2) {
        bool hit = false;
        for (int i = 0; i < paddleSize; i++) {
            if (ballY == rightPaddleY + i) {
                hit = true;
                break;
            }
        }
        if (hit) {
            ballSpeedX = -ballSpeedX;
            if (ballSpeedY == 0) ballSpeedY = (rand() % 2) ? 1 : -1; // Добавление случайности для избежания предсказуемости поведения мяча
        } else if (ballX >= WIDTH - 1) {
            leftScore++;
            ballX = WIDTH / 2;
            ballY = HEIGHT / 2;
            ballSpeedX = -ballSpeedX;
        }
    }
}

// Отображение начального меню
void ShowMenu() {
    system("cls");
    cout << "===== PONG GAME =====" << endl;
    cout << "Select difficulty:" << endl;
    cout << "1. Easy (slow ball, large paddle)" << endl;
    cout << "2. Medium (medium ball, medium paddle)" << endl;
    cout << "3. Hard (fast ball, small paddle)" << endl;
    cout << endl;
    cout << "Controls:" << endl;
    cout << "Player 1: W (up), S (down)" << endl;
    cout << "Player 2: Arrow Up, Arrow Down" << endl;
    cout << "Quit: Q" << endl;
    cout << endl;
    cout << "Enter your choice (1-3): ";

    int choice;
    cin >> choice;

    switch (choice) {
        case 1:
            currentDifficulty = EASY;
            gameSpeed = 50;
            paddleSize = 5;
            break;
        case 2:
            currentDifficulty = MEDIUM;
            gameSpeed = 30;
            paddleSize = 3;
            break;
        case 3:
            currentDifficulty = HARD;
            gameSpeed = 15;
            paddleSize = 2;
            break;
        default:
            currentDifficulty = MEDIUM;
            gameSpeed = 30;
            paddleSize = 3;
            break;
    }
}

// Инициализация консоли с установленными параметрами
void InitializeConsole() {
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);
    MoveWindow(console, r.left, r.top, 800, 600, TRUE);

    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// Инициализация игры; систематизация всех остальных функций
int main() {
    InitializeConsole();
    ShowMenu();
    SetupGame();

    auto lastTime = chrono::steady_clock::now();
    while (gameRunning) {
        auto currentTime = chrono::steady_clock::now();
        auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentTime - lastTime).count();

        if (elapsedTime >= 16) {
            Input();
            Logic();
            Draw();
            lastTime = currentTime;
        }

        this_thread::sleep_for(chrono::milliseconds(gameSpeed));
    }

    system("cls");
    cout << "Game Over!" << endl;
    cout << "Final Score: " << leftScore << " - " << rightScore << endl;
    if (leftScore > rightScore) {
        cout << "Player 1 wins!" << endl;
    } else if (rightScore > leftScore) {
        cout << "Player 2 wins!" << endl;
    } else {
        cout << "It's a tie!" << endl;
    }

    cout << "Press any key to exit...";
    _getch();

    return 0;
}
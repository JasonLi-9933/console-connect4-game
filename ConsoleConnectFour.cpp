// ConsoleConnectFour.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <thread>

using namespace std;

int nScreenWidth = 80;
int nScreenHeight = 30;
int nFieldWidth = 15;
int nFieldHeight = 22;
int nFieldOffset = 2;
int nCursorHeight = nFieldHeight - 1;

const wstring RENDER_CHARS = L" XO#^";
const int PLAYER_1 = 5;
const int PLAYER_2 = 6;
unsigned char* pField = nullptr;
int* HeightRecord;

void InitPlayField()
{
    pField = new unsigned char[nFieldHeight * nFieldWidth];
    for (int x = 0; x < nFieldWidth; x++)
    {
        for (int y = 0; y < nFieldHeight; y++)
        {
            bool atBoundary = x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1;
            pField[y * nFieldWidth + x] = atBoundary ? 3 : 0;
        }
    }
}

void InitHeightRecord()
{
    HeightRecord = new int[nFieldWidth];
    for (int i = 0; i < nFieldWidth; i++)
    {
        HeightRecord[i] = nFieldHeight - 2;
    }
}

void DrawPlayField(wchar_t* screen)
{
	for (int x = 0; x < nFieldWidth; x++)
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			wchar_t c = RENDER_CHARS[pField[y * nFieldWidth + x]];
			screen[(y + nFieldOffset) * nScreenWidth + (x + nFieldOffset)] = RENDER_CHARS[pField[y * nFieldWidth + x]];
		}
	}
}

void InitScreen(wchar_t* screen)
{
    for (int i = 0; i < nScreenHeight * nScreenWidth; i++)
    {
        screen[i] = L' ';
    }
}

bool WithinPlayField(int cursorX)
{
    return cursorX >= 1 && cursorX <= nFieldWidth - 2;
}

int Left(int y, int x)
{
    if (x <= 1) return -1;
    return y * nFieldWidth + x - 1;;
}

int Right(int y, int x)
{
    if (x >= nFieldWidth - 2) return -1;
    return y * nFieldWidth + x + 1;
}

int Top(int y, int x)
{
    if (y <= 0) return -1;
    return (y - 1) * nFieldWidth + x;
}

int Bottom(int y, int x)
{
    if (y >= nFieldHeight - 2) return -1;
    return (y + 1) * nFieldWidth + x;
}

int TopLeft(int y, int x)
{
    if (y <= 0 || x <= 1) return -1;
    return (y - 1) * nFieldWidth + x - 1;
}

int TopRight(int y, int x)
{
    if (y <= 0 || x >= nFieldWidth - 2) return -1;
    return (y - 1) * nFieldWidth + x + 1;
}

int BottomLeft(int y, int x)
{
    if (y >= nFieldHeight - 2 || x <= 1) return -1;
    return (y + 1) * nFieldWidth + x - 1;
}

int BottomRight(int y, int x)
{
    if (y >= nFieldHeight - 2 || x >= nFieldWidth - 2) return -1;
    return (y + 1) * nFieldWidth + x + 1;
}

bool CheckOneDirection(int y, int x, int yDir, int xDir, int player, int(*GetNextIndex)(int, int))
{
    for (int i = 1; i <= 3; i++)
    {
        int index = GetNextIndex(y, x);
        if (index >= 0 && pField[index] == player) {
            y += yDir;
            x += xDir;
        }
        else {
            return false;
        }
    }
    return true;
}

bool FourPiecesConnected(int y, int x, int player)
{
    // Check top-left direction
    if (CheckOneDirection(y, x, -1, -1, player, TopLeft)) return true;

    // Check top direction
    if (CheckOneDirection(y, x, -1, 0, player, Top)) return true;

    // Check top-right direction
    if (CheckOneDirection(y, x, -1, 1, player, TopRight)) return true;

    // Check left direction
    if (CheckOneDirection(y, x, 0, -1, player, Left)) return true;

    // Check right direction
    if (CheckOneDirection(y, x, 0, 1, player, Right)) return true;

    // Check bottom left direction
    if (CheckOneDirection(y, x, 1, -1, player, BottomLeft)) return true;

    // Check bottom direction
    if (CheckOneDirection(y, x, 1, 0, player, Bottom)) return true;

    // Check bottom right direction
    if (CheckOneDirection(y, x, 1, 1, player, BottomRight)) return true;

    return false;
}

int main()
{
    InitPlayField();
    wchar_t* screen = new wchar_t[nScreenHeight * nScreenWidth];
    InitScreen(screen);
    InitHeightRecord();

    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Game Logic Variables
    int nCursorX = nFieldWidth / 2;
    int currentPlayer = 1; // 1 or 2
    bool bKey[4];
    const int RIGHT_KEY = 0;
    const int LEFT_KEY = 1;
    const int PLAYER1_KEY = 2;
    const int PLAYER2_KEY = 3;


    bool GameOver = false;
    while (!GameOver) 
    {
        // Game Timing
        this_thread::sleep_for(50ms);
        swprintf_s(&screen[nFieldOffset], 17, L"Current Player:%1d", currentPlayer);
        swprintf_s(&screen[nScreenWidth + nFieldOffset], 25, L"Player 1: X, Player 2: O");
        for (int k = 0; k < 4; k++)
        {
            // virtual key code: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x31\x32"[k]))) != 0;
        }
        nCursorX -= (bKey[LEFT_KEY] && WithinPlayField(nCursorX - 1)) ? 1 : 0;
        nCursorX += (bKey[RIGHT_KEY] && WithinPlayField(nCursorX + 1)) ? 1 : 0;

        if (bKey[PLAYER1_KEY] && currentPlayer == 1) {
            int height = HeightRecord[nCursorX];
            if (height >= 0) {
                pField[height * nFieldWidth + nCursorX] = currentPlayer;
                HeightRecord[nCursorX] = height - 1;
                // Check win condition OR Change player
                if (FourPiecesConnected(height, nCursorX, currentPlayer)) {
                    GameOver = true;
                }
                else {
					currentPlayer = 2;
                }
            }
        } else if (bKey[PLAYER2_KEY] && currentPlayer == 2) {
            int height = HeightRecord[nCursorX];
            if (height >= 0) {
                pField[height * nFieldWidth + nCursorX] = currentPlayer;
                HeightRecord[nCursorX] = height - 1;
                // Check win condition OR Change player
                if (FourPiecesConnected(height, nCursorX, currentPlayer)) {
                    GameOver = true;
                }
                else {
					currentPlayer = 1;
                }
            }
        }

        // Draw Playing Field
        DrawPlayField(screen);

        // Draw Cursor
        screen[(nCursorHeight + nFieldOffset) * nScreenWidth + (nCursorX + nFieldOffset)] = RENDER_CHARS[currentPlayer];
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
    }

    CloseHandle(hConsole);
    cout << "Game Over!!! Winnder is Player " << currentPlayer << endl;
    system("pause");
}

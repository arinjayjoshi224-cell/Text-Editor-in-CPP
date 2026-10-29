#include "ConsoleHelpers.h"

void setTextColor(WORD color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void resetTextColor()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(
        hConsole,
        FOREGROUND_RED |
        FOREGROUND_GREEN |
        FOREGROUND_BLUE
    );
}

void gotoxy(int x, int y)
{
    COORD pos;
    pos.X = x;
    pos.Y = y;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pos);
}

// Swallow Ctrl+C so it can be used as "copy" instead of killing the program.
BOOL WINAPI ctrlHandler(DWORD type)
{
    return type == CTRL_C_EVENT;
}

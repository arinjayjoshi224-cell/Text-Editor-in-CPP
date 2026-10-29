#include "TextEditor.h"
#include "FileHandler.h"
#include "ConsoleHelpers.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

void TextEditor::flushBuffer(HANDLE hOut, string& buf)
{
    if (buf.empty())
        return;

    DWORD written;

    WriteConsoleA(hOut, buf.c_str(), (DWORD)buf.size(), &written, NULL);

    buf.clear();
}

void TextEditor::clearScreen(HANDLE hOut)
{
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        if (!GetConsoleScreenBufferInfo(hOut, &csbi))
            return;

        DWORD cells = (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y;
        DWORD written;
        COORD home = { 0, 0 };

        FillConsoleOutputCharacterA(hOut, ' ', cells, home, &written);
        FillConsoleOutputAttribute(hOut, csbi.wAttributes, cells, home, &written);
        SetConsoleCursorPosition(hOut, home);
    }


void TextEditor::display()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    clearScreen(hOut);

    const WORD normalAttr =
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

    const WORD selectedAttr =
        BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;

    // Light grey line-number colour
    const WORD lineNumberAttr =
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
        FOREGROUND_INTENSITY;

    int sl = 0, sc = 0, el = 0, ec = 0;
    bool selecting = getSelectionRange(sl, sc, el, ec);

    string buf;
    WORD curAttr = normalAttr;

    LineNode* line = firstLine;
    int row = 0;

    while (line != NULL)
    {
        // Display line number in light grey
        SetConsoleTextAttribute(hOut, lineNumberAttr);

        COORD pos;
        pos.X = 0;
        pos.Y = row;
        SetConsoleCursorPosition(hOut, pos);

        string lineNumber = to_string(row + 1) + "  ";
        DWORD written;

        WriteConsoleA(
            hOut,
            lineNumber.c_str(),
            (DWORD)lineNumber.size(),
            &written,
            NULL
        );
        
        // Display actual text in normal/selection colour
        SetConsoleTextAttribute(hOut, normalAttr);
        curAttr = normalAttr;

        CharNode* temp = line->head;
        int col = 0;

        while (temp != NULL)
        {
            bool selected = false;

            if (selecting)
            {
                bool afterStart =
                    row > sl || (row == sl && col >= sc);

                bool beforeEnd =
                    row < el || (row == el && col < ec);

                selected = afterStart && beforeEnd;
            }

            WORD attr = selected ? selectedAttr : normalAttr;

            if (attr != curAttr)
            {
                flushBuffer(hOut, buf);
                SetConsoleTextAttribute(hOut, attr);
                curAttr = attr;
            }

            buf += temp->data;

            temp = temp->next;
            col++;
        }

        flushBuffer(hOut, buf);

        SetConsoleTextAttribute(hOut, normalAttr);
        curAttr = normalAttr;

        buf += '\r';
        buf += '\n';

        flushBuffer(hOut, buf);

        line = line->next;
        row++;
    }

    flushBuffer(hOut, buf);

    // Restore normal text colour
    SetConsoleTextAttribute(hOut, normalAttr);

    // Keep cursor at its original document position
    int x = getCurrentColumn() + 4;
    int y = currentLineNumber;

    gotoxy(x, y);
}

void TextEditor::showList()
{
    LineNode* line =
        firstLine;

    while (line != NULL)
    {
        CharNode* temp =
            line->head;

        while (temp != NULL)
        {
            cout << temp->data;

            temp = temp->next;
        }

        cout << endl;

        line = line->next;
    }
}


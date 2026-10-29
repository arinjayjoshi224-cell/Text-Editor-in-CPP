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
        FOREGROUND_GREEN | FOREGROUND_INTENSITY;

    const WORD lineNumberAttr =
        FOREGROUND_RED | FOREGROUND_GREEN;

    const WORD selectedAttr =
        BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;

    const int gutterWidth = 5;

    int sl = 0, sc = 0, el = 0, ec = 0;
    bool selecting = getSelectionRange(sl, sc, el, ec);

    string buf;
    WORD curAttr = normalAttr;

    LineNode* line = firstLine;
    int row = 0;

    while (line != NULL)
    {
        // Display the line number in light gray.
        flushBuffer(hOut, buf);
        SetConsoleTextAttribute(hOut, lineNumberAttr);

        string number = to_string(row + 1);
        int padding = gutterWidth - 1 - (int)number.length();

        if (padding < 0)
            padding = 0;

        string prefix(padding, ' ');
        prefix += number;
        prefix += " ";

        DWORD written;
        WriteConsoleA(hOut, prefix.c_str(),
            (DWORD)prefix.size(), &written, NULL);

        // Restore the document text color.
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

            WORD attr = selected ? selectedAttr : temp->color;

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

        if (curAttr != normalAttr)
        {
            flushBuffer(hOut, buf);
            SetConsoleTextAttribute(hOut, normalAttr);
            curAttr = normalAttr;
        }

        buf += '\n';

        line = line->next;
        row++;
    }

    flushBuffer(hOut, buf);

    // Position the cursor after the line-number gutter.
    int x = getCurrentColumn() + gutterWidth;
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


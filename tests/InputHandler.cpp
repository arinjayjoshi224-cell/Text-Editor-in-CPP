#include "TextEditor.h"
#include "FileHandler.h"
#include "ConsoleHelpers.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

void TextEditor::start()
{
    HANDLE hInput =
        GetStdHandle(STD_INPUT_HANDLE);

    DWORD mode;

    GetConsoleMode(
        hInput,
        &mode
    );

    // No ENABLE_PROCESSED_INPUT: Ctrl+C arrives as a normal key event.
    SetConsoleMode(
        hInput,
        ENABLE_EXTENDED_FLAGS |
        ENABLE_WINDOW_INPUT
    );

    // Belt and braces: if the terminal still raises a Ctrl+C signal,
    // swallow it instead of letting it kill the program.
    SetConsoleCtrlHandler(NULL, TRUE);          // ignore Ctrl+C outright
    SetConsoleCtrlHandler(ctrlHandler, TRUE);

    HANDLE hOutput =
        GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursorInfo;

    cursorInfo.dwSize = 20;
    cursorInfo.bVisible = TRUE;

    SetConsoleCursorInfo(
        hOutput,
        &cursorInfo
    );

    INPUT_RECORD input;

    DWORD events;

    bool dirty = true;

    while (true)
    {
        if (dirty)
        {
            display();
            dirty = false;
        }

        ReadConsoleInput(
            hInput,
            &input,
            1,
            &events
        );

        if (input.EventType != KEY_EVENT)
            continue;

        KEY_EVENT_RECORD key =
            input.Event.KeyEvent;

        if (!key.bKeyDown)
            continue;

        dirty = true;       // redraw only after real key presses

        bool ctrl =
            (key.dwControlKeyState &
                (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;

        bool shift =
            (key.dwControlKeyState & SHIFT_PRESSED) != 0;

        WORD vk = key.wVirtualKeyCode;

        if (vk == VK_ESCAPE)
        {
            break;
        }
        
        
        else if (ctrl && shift && vk == 'S')
        {
            // Ctrl + Shift + S: Save As
            saveDocument(true);
        }
        else if (ctrl && vk == 'S')
        {
            // Ctrl + S: Save current file
            saveDocument(false);
        }
        else if (ctrl && vk == 'O')
        {
            // Ctrl + O: Open a specific file
            openDocument();
        }

        else if (ctrl && vk == 'Z')
        {
           undo();
        }
        else if (ctrl && vk == 'Y')
        {
           redo();
        }

        // Ctrl+C is also matched by its control character (ETX, 3) in
        // case the terminal reports a different virtual-key code.
        else if ((ctrl && vk == 'C') || key.uChar.AsciiChar == 3 ||
                    (ctrl && vk == VK_INSERT))
        {
            copy();
        }
        else if ((ctrl && vk == 'X') || (shift && vk == VK_DELETE))
        {
            cut();
        }
        else if ((ctrl && vk == 'V') || (shift && vk == VK_INSERT))
        {
            paste();
        }
        else if (ctrl && vk == 'A')
        {
            selectAll();
        }
        else if (vk == VK_LEFT || vk == VK_RIGHT ||
                    vk == VK_UP || vk == VK_DOWN)
        {
            int sl, sc, el, ec;

            if (shift)
            {
                beginSelection();
            }
            else if (getSelectionRange(sl, sc, el, ec))
            {
                // Left/Right collapse the selection to its edge
                if (vk == VK_LEFT)
                {
                    clearSelection();
                    moveTo(sl, sc);
                    continue;
                }
                else if (vk == VK_RIGHT)
                {
                    clearSelection();
                    moveTo(el, ec);
                    continue;
                }

                clearSelection();
            }
            else
            {
                clearSelection();
            }

            if (vk == VK_LEFT)
                moveLeft();
            else if (vk == VK_RIGHT)
                moveRight();
            else if (vk == VK_UP)
                moveUp();
            else
                moveDown();
        }
        else if (vk == VK_BACK)
        {
            if (!deleteSelection())
                backspace();
        }
        else if (vk == VK_DELETE)
        {
            if (!deleteSelection())
                deleteChar();
        }
        else if (vk == VK_RETURN)
        {
            deleteSelection();
            newLine();
        }
        else if (!ctrl &&
                    key.uChar.AsciiChar >= 32 &&
                    key.uChar.AsciiChar <= 126)
        {
            deleteSelection();      // typing replaces selection
            insert(key.uChar.AsciiChar);
        }
    }

    SetConsoleCtrlHandler(ctrlHandler, FALSE);
    SetConsoleCtrlHandler(NULL, FALSE);
    SetConsoleMode(hInput, mode);
}


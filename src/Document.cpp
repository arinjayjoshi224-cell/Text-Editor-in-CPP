#include "TextEditor.h"
#include "FileHandler.h"
#include "ConsoleHelpers.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

string TextEditor::normalizeFilename(string filename)
{
        if (filename.empty())
            return filename;

        // Only inspect the filename, not dots in the directory path.
        size_t slash = filename.find_last_of("\\/");
        size_t dot = filename.find_last_of('.');

        if (dot == string::npos ||
            (slash != string::npos && dot < slash))
        {
            filename += ".txt";
        }

        return filename;
    }

string TextEditor::readFilename()
{
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

        INPUT_RECORD input;
        DWORD events;

        string filename;

        while (true)
        {
            if (!ReadConsoleInput(hInput, &input, 1, &events))
                break;

            if (input.EventType != KEY_EVENT)
                continue;

            KEY_EVENT_RECORD key = input.Event.KeyEvent;

            if (!key.bKeyDown)
                continue;

            WORD vk = key.wVirtualKeyCode;
            char ch = key.uChar.AsciiChar;

            if (vk == VK_RETURN)
            {
                cout << '\n';
                break;
            }

            if (vk == VK_ESCAPE)
            {
                filename.clear();
                cout << '\n';
                break;
            }

            if (vk == VK_BACK)
            {
                if (!filename.empty())
                {
                    filename.pop_back();
                    cout << "\b \b";
                }

                continue;
            }

            bool ctrl = (key.dwControlKeyState &
                (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;

            if (!ctrl && ch >= 32 && ch <= 126)
            {
                filename += ch;
                cout << ch;
            }
        }

        return filename;
    }

bool TextEditor::openDocument()
{
    string filename;

    if (!chooseOpenFilename(filename))
    {
        cout << "\nOpen cancelled.\n";
        return false;
    }

    string content;

    if (!loadFromFile(filename, content))
    {
        cout << "\nCould not open file: "
             << filename << "\n";
        return false;
    }

    setText(content);
    currentFilename = filename;

    cout << "\nOpened successfully: "
         << currentFilename << "\n";

    return true;
}

string TextEditor::getText()
{
        string content;

        LineNode* line = firstLine;

        while (line != NULL)
        {
            CharNode* ch = line->head;

            while (ch != NULL)
            {
                content += ch->data;
                ch = ch->next;
            }

            if (line->next != NULL)
                content += '\n';

            line = line->next;
        }

        return content;
    }

void TextEditor::setText(const string& content)
{
        // Delete all existing lines and characters.
        LineNode* line = firstLine;

        while (line != NULL)
        {
            CharNode* ch = line->head;

            while (ch != NULL)
            {
                CharNode* nextChar = ch->next;
                delete ch;
                ch = nextChar;
            }

            LineNode* nextLine = line->next;
            delete line;
            line = nextLine;
        }

        // Start with one empty line.
        firstLine = new LineNode();
        lastLine = firstLine;
        currentLine = firstLine;
        cursor = NULL;
        currentLineNumber = 0;

        clearSelection();

        // Rebuild the linked list from the loaded string.
        for (size_t i = 0; i < content.size(); ++i)
        {
            char ch = content[i];

            if (ch == '\n')
            {
                LineNode* newNode = new LineNode();

                newNode->prev = lastLine;
                lastLine->next = newNode;
                lastLine = newNode;
            }
            else if (ch == '\t')
            {
                for (int k = 0; k < 4; ++k)
                    appendChar(lastLine, ' ');
            }
            else if (ch >= 32 && ch <= 126)
            {
                appendChar(lastLine, ch);
            }
        }

        // Place the cursor at the beginning.
        currentLine = firstLine;
        currentLineNumber = 0;
        cursor = firstLine->head;

        // Update line lookup after rebuilding the document.
        rebuildSkipList();
    }


bool TextEditor::saveDocument(bool saveAs)
{
        string filename = currentFilename;

        if (saveAs || filename.empty())
        {
            cout << "\nEnter filename or full path: ";

            filename = readFilename();

            if (filename.empty())
            {
                cout << "Save cancelled.\n";
                return false;
            }

            filename = normalizeFilename(filename);
        }

        if (saveToFile(filename, getText()))
        {
            currentFilename = filename;

            cout << "\nSaved successfully: "
                << currentFilename << "\n";

            return true;
        }

        cout << "\nCould not save file: "
            << filename << "\n";

        return false;
    }

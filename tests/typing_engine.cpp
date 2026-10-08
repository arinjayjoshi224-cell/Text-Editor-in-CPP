#include <iostream>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

void gotoxy(int x, int y)
{
    COORD pos;
    pos.X = x;
    pos.Y = y;

    SetConsoleCursorPosition(
        GetStdHandle(STD_OUTPUT_HANDLE),
        pos
    );
}

struct CharNode
{
    char data;
    CharNode* prev;
    CharNode* next;

    CharNode(char ch)
    {
        data = ch;
        prev = NULL;
        next = NULL;
    }
};

struct LineNode
{
    CharNode* head;
    CharNode* tail;

    LineNode* prev;
    LineNode* next;

    LineNode()
    {
        head = NULL;
        tail = NULL;
        prev = NULL;
        next = NULL;
    }
};

const int MAX_LEVEL = 4;

struct SkipNode
{
    LineNode* line;
    int lineNumber;

    SkipNode* forward[MAX_LEVEL];

    SkipNode(LineNode* l, int number)
    {
        line = l;
        lineNumber = number;

        for (int i = 0; i < MAX_LEVEL; i++)
            forward[i] = NULL;
    }
};

class TextEditor
{
private:

    LineNode* firstLine;
    LineNode* lastLine;
    LineNode* currentLine;

    CharNode* cursor;

    int currentLineNumber;

    SkipNode* skipHead;
    int skipLevel;

    void clearSkipList()
    {
        if (skipHead == NULL)
            return;

        SkipNode* current = skipHead;

        while (current != NULL)
        {
            SkipNode* next = current->forward[0];

            delete current;

            current = next;
        }

        skipHead = NULL;
        skipLevel = 1;
    }

    void rebuildSkipList()
    {
        clearSkipList();

        vector<LineNode*> lines;

        LineNode* line = firstLine;

        while (line != NULL)
        {
            lines.push_back(line);
            line = line->next;
        }

        if (lines.empty())
            return;

        vector<SkipNode*> nodes;

        for (int i = 0; i < (int)lines.size(); i++)
        {
            nodes.push_back(
                new SkipNode(lines[i], i)
            );
        }

        for (int i = 0; i < (int)nodes.size() - 1; i++)
        {
            nodes[i]->forward[0] =
                nodes[i + 1];
        }

        for (int level = 1;
             level < MAX_LEVEL;
             level++)
        {
            int step = 1 << level;

            for (int i = 0;
                 i + step < (int)nodes.size();
                 i += step)
            {
                nodes[i]->forward[level] =
                    nodes[i + step];
            }
        }

        skipHead = nodes[0];

        skipLevel = MAX_LEVEL;
    }

    LineNode* getLine(int lineNumber)
    {
        if (skipHead == NULL)
            return NULL;

        if (lineNumber < 0)
            return NULL;

        SkipNode* current = skipHead;

        for (int level = MAX_LEVEL - 1;
             level >= 0;
             level--)
        {
            while (
                current->forward[level] != NULL &&
                current->forward[level]->lineNumber <= lineNumber
            )
            {
                current =
                    current->forward[level];
            }
        }

        if (current->lineNumber == lineNumber)
            return current->line;

        return NULL;
    }

    int getCurrentColumn()
    {
        int column = 0;

        CharNode* temp = currentLine->head;

        while (temp != NULL)
        {
            if (temp == cursor)
                return column;

            column++;

            temp = temp->next;
        }

        return column;
    }

    int getLineLength(LineNode* line)
    {
        int length = 0;

        CharNode* temp = line->head;

        while (temp != NULL)
        {
            length++;

            temp = temp->next;
        }

        return length;
    }

    void setCursorColumn(int column)
    {
        if (column <= 0)
        {
            cursor = currentLine->head;
            return;
        }

        CharNode* temp = currentLine->head;

        int currentColumn = 0;

        while (
            temp != NULL &&
            currentColumn < column
        )
        {
            temp = temp->next;
            currentColumn++;
        }

        cursor = temp;
    }

    void updateCurrentLineNumber()
    {
        currentLineNumber = 0;

        LineNode* temp = firstLine;

        while (temp != NULL)
        {
            if (temp == currentLine)
                return;

            currentLineNumber++;

            temp = temp->next;
        }
    }

public:

    TextEditor()
    {
        srand((unsigned)time(NULL));

        firstLine = new LineNode();

        lastLine = firstLine;
        currentLine = firstLine;

        cursor = NULL;

        currentLineNumber = 0;

        skipHead = NULL;
        skipLevel = 1;

        rebuildSkipList();
    }

    void insert(char ch)
    {
        CharNode* newNode = new CharNode(ch);

        if (currentLine->head == NULL)
        {
            currentLine->head = newNode;
            currentLine->tail = newNode;

            cursor = NULL;

            return;
        }

        if (cursor == NULL)
        {
            newNode->prev = currentLine->tail;

            currentLine->tail->next =
                newNode;

            currentLine->tail =
                newNode;

            return;
        }

        CharNode* left = cursor->prev;

        newNode->next = cursor;
        newNode->prev = left;

        cursor->prev = newNode;

        if (left != NULL)
        {
            left->next = newNode;
        }
        else
        {
            currentLine->head = newNode;
        }
    }

    void moveLeft()
    {
        if (cursor != NULL)
        {
            if (cursor->prev != NULL)
            {
                cursor = cursor->prev;
            }
            else
            {
                if (currentLine->prev != NULL)
                {
                    currentLine =
                        currentLine->prev;

                    updateCurrentLineNumber();

                    cursor = NULL;
                }
            }

            return;
        }

        if (currentLine->tail != NULL)
        {
            cursor = currentLine->tail;
        }
        else
        {
            if (currentLine->prev != NULL)
            {
                currentLine =
                    currentLine->prev;

                updateCurrentLineNumber();

                cursor = NULL;
            }
        }
    }

    void moveRight()
    {
        if (cursor != NULL)
        {
            if (cursor->next != NULL)
            {
                cursor = cursor->next;
            }
            else
            {
                if (currentLine->next != NULL)
                {
                    currentLine =
                        currentLine->next;

                    updateCurrentLineNumber();

                    cursor =
                        currentLine->head;
                }
                else
                {
                    cursor = NULL;
                }
            }

            return;
        }

        if (currentLine->next != NULL)
        {
            currentLine =
                currentLine->next;

            updateCurrentLineNumber();

            cursor =
                currentLine->head;
        }
    }

    void backspace()
    {
        if (currentLine->head == NULL)
        {
            if (currentLine->prev != NULL)
            {
                mergeWithPreviousLine();
            }

            return;
        }

        if (cursor == currentLine->head)
        {
            if (currentLine->prev != NULL)
            {
                mergeWithPreviousLine();
            }

            return;
        }

        if (cursor == NULL)
        {
            CharNode* del =
                currentLine->tail;

            currentLine->tail =
                del->prev;

            if (currentLine->tail != NULL)
            {
                currentLine->tail->next =
                    NULL;
            }
            else
            {
                currentLine->head = NULL;
            }

            delete del;

            return;
        }

        CharNode* del =
            cursor->prev;

        if (del == NULL)
            return;

        CharNode* left =
            del->prev;

        cursor->prev = left;

        if (left != NULL)
        {
            left->next = cursor;
        }
        else
        {
            currentLine->head = cursor;
        }

        delete del;
    }

    void deleteChar()
    {
        if (cursor == NULL)
        {
            if (currentLine->next != NULL)
            {
                mergeWithNextLine();
            }

            return;
        }

        CharNode* del = cursor;

        cursor = cursor->next;

        if (del->prev != NULL)
        {
            del->prev->next = cursor;
        }
        else
        {
            currentLine->head = cursor;
        }

        if (cursor != NULL)
        {
            cursor->prev = del->prev;
        }
        else
        {
            currentLine->tail =
                del->prev;
        }

        delete del;
    }

    void mergeWithPreviousLine()
    {
        LineNode* previous = currentLine->prev;

        if (previous == NULL)
            return;

        CharNode* firstOfCurrent = currentLine->head;

        if (currentLine->head == NULL)
        {
            previous->next = currentLine->next;

            if (currentLine->next != NULL)
            {
                currentLine->next->prev = previous;
            }
            else
            {
                lastLine = previous;
            }

            delete currentLine;

            currentLine = previous;

            cursor = NULL;

            updateCurrentLineNumber();

            rebuildSkipList();

            return;
        }

        if (previous->head == NULL)
        {
            previous->head = currentLine->head;
            previous->tail = currentLine->tail;
        }
        else
        {
            previous->tail->next = currentLine->head;
            currentLine->head->prev = previous->tail;
            previous->tail = currentLine->tail;
        }

        previous->next = currentLine->next;

        if (currentLine->next != NULL)
        {
            currentLine->next->prev = previous;
        }
        else
        {
            lastLine = previous;
        }

        delete currentLine;

        currentLine = previous;

        cursor = firstOfCurrent;

        updateCurrentLineNumber();

        rebuildSkipList();
    }
    
    void mergeWithNextLine()
    {
        LineNode* nextLine =
            currentLine->next;

        if (nextLine == NULL)
            return;

        if (currentLine->head == NULL)
        {
            currentLine->head =
                nextLine->head;

            currentLine->tail =
                nextLine->tail;
        }
        else if (nextLine->head != NULL)
        {
            currentLine->tail->next =
                nextLine->head;

            nextLine->head->prev =
                currentLine->tail;

            currentLine->tail =
                nextLine->tail;
        }

        currentLine->next =
            nextLine->next;

        if (nextLine->next != NULL)
        {
            nextLine->next->prev =
                currentLine;
        }
        else
        {
            lastLine = currentLine;
        }

        delete nextLine;

        cursor = NULL;

        rebuildSkipList();
    }

    void newLine()
    {
        LineNode* newLine =
            new LineNode();

        if (cursor == NULL)
        {
            newLine->prev =
                currentLine;

            newLine->next =
                currentLine->next;

            if (currentLine->next != NULL)
            {
                currentLine->next->prev =
                    newLine;
            }
            else
            {
                lastLine = newLine;
            }

            currentLine->next =
                newLine;

            currentLine =
                newLine;

            cursor = NULL;

            updateCurrentLineNumber();

            rebuildSkipList();

            return;
        }

        newLine->head = cursor;
        newLine->tail =
            currentLine->tail;

        CharNode* left =
            cursor->prev;

        if (left != NULL)
        {
            left->next = NULL;

            currentLine->tail =
                left;
        }
        else
        {
            currentLine->head = NULL;
            currentLine->tail = NULL;
        }

        cursor->prev = NULL;

        newLine->prev =
            currentLine;

        newLine->next =
            currentLine->next;

        if (currentLine->next != NULL)
        {
            currentLine->next->prev =
                newLine;
        }
        else
        {
            lastLine = newLine;
        }

        currentLine->next =
            newLine;

        currentLine =
            newLine;

        cursor =
            newLine->head;

        updateCurrentLineNumber();

        rebuildSkipList();
    }

    void moveUp()
    {
        if (currentLineNumber == 0)
            return;

        int column =
            getCurrentColumn();

        int targetLineNumber =
            currentLineNumber - 1;

        LineNode* target =
            getLine(targetLineNumber);

        if (target == NULL)
            return;

        currentLine = target;

        currentLineNumber =
            targetLineNumber;

        int length =
            getLineLength(currentLine);

        column =
            min(column, length);

        setCursorColumn(column);
    }

    void moveDown()
    {
        int column =
            getCurrentColumn();

        int targetLineNumber =
            currentLineNumber + 1;

        LineNode* target =
            getLine(targetLineNumber);

        if (target == NULL)
            return;

        currentLine = target;

        currentLineNumber =
            targetLineNumber;

        int length =
            getLineLength(currentLine);

        column =
            min(column, length);

        setCursorColumn(column);
    }

    void display()
    {
        system("cls");

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

            cout << '\n';

            line = line->next;
        }

        int x = getCurrentColumn();
        int y = currentLineNumber;

        gotoxy(x, y);
    }

    void start()
    {
        HANDLE hInput =
            GetStdHandle(STD_INPUT_HANDLE);

        DWORD mode;

        GetConsoleMode(
            hInput,
            &mode
        );

        SetConsoleMode(
            hInput,
            ENABLE_EXTENDED_FLAGS
        );

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

        while (true)
        {
            display();

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

            if (key.wVirtualKeyCode == VK_ESCAPE)
            {
                break;
            }
            else if (
                key.wVirtualKeyCode == VK_LEFT
            )
            {
                moveLeft();
            }
            else if (
                key.wVirtualKeyCode == VK_RIGHT
            )
            {
                moveRight();
            }
            else if (
                key.wVirtualKeyCode == VK_UP
            )
            {
                moveUp();
            }
            else if (
                key.wVirtualKeyCode == VK_DOWN
            )
            {
                moveDown();
            }
            else if (
                key.wVirtualKeyCode == VK_BACK
            )
            {
                backspace();
            }
            else if (
                key.wVirtualKeyCode == VK_DELETE
            )
            {
                deleteChar();
            }
            else if (
                key.wVirtualKeyCode == VK_RETURN
            )
            {
                newLine();
            }
            else if (
                key.uChar.AsciiChar >= 32 &&
                key.uChar.AsciiChar <= 126
            )
            {
                insert(
                    key.uChar.AsciiChar
                );
            }
        }
    }

    void showList()
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
};

int main()
{
    TextEditor editor;

    editor.start();

    system("cls");

    cout << "Final Text:\n\n";

    editor.showList();

    return 0;
}
#include <iostream>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>

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


struct Position
{
    LineNode* line;
    CharNode* cursor;

    Position()
    {
        line = NULL;
        cursor = NULL;
    }

    Position(LineNode* l, CharNode* c)
    {
        line = l;
        cursor = c;
    }
};

struct Clipboard
{
    LineNode* first;
    LineNode* last;

    Clipboard()
    {
        first = NULL;
        last = NULL;
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


    Position selectionStart;
    Position selectionEnd;
    bool selecting;

    Clipboard clipboard;

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


    Position getCursorPosition()
    {
        return Position(currentLine, cursor);
    }

    bool samePosition(const Position& a, const Position& b)
    {
        return a.line == b.line &&
               a.cursor == b.cursor;
    }

    int getColumn(LineNode* line, CharNode* node)
    {
        int column = 0;
        CharNode* temp = line->head;

        while (temp != NULL)
        {
            if (temp == node)
                return column;

            column++;
            temp = temp->next;
        }

        // NULL means the position is at the end of the line.
        return column;
    }

    int getLineNumber(LineNode* target)
    {
        int number = 0;
        LineNode* temp = firstLine;

        while (temp != NULL)
        {
            if (temp == target)
                return number;

            number++;
            temp = temp->next;
        }

        return -1;
    }

    bool positionBefore(const Position& a, const Position& b)
    {
        int lineA = getLineNumber(a.line);
        int lineB = getLineNumber(b.line);

        if (lineA != lineB)
            return lineA < lineB;

        return getColumn(a.line, a.cursor) <
               getColumn(b.line, b.cursor);
    }

    void normalizeSelection(Position& start, Position& end)
    {
        if (!positionBefore(start, end))
        {
            Position temp = start;
            start = end;
            end = temp;
        }
    }

    void movePositionForward(Position& p)
    {
        if (p.line == NULL)
            return;

        if (p.cursor != NULL)
        {
            if (p.cursor->next != NULL)
            {
                p.cursor = p.cursor->next;
            }
            else if (p.line->next != NULL)
            {
                p.line = p.line->next;
                p.cursor = p.line->head;
            }
            else
            {
                p.cursor = NULL;
            }
        }
        else if (p.line->next != NULL)
        {
            p.line = p.line->next;
            p.cursor = p.line->head;
        }
    }

    void clearClipboard()
    {
        LineNode* line = clipboard.first;

        while (line != NULL)
        {
            LineNode* nextLine = line->next;

            CharNode* ch = line->head;
            while (ch != NULL)
            {
                CharNode* nextChar = ch->next;
                delete ch;
                ch = nextChar;
            }

            delete line;
            line = nextLine;
        }

        clipboard.first = NULL;
        clipboard.last = NULL;
    }

    LineNode* addClipboardLine()
    {
        LineNode* newLine = new LineNode();

        if (clipboard.first == NULL)
        {
            clipboard.first = newLine;
            clipboard.last = newLine;
        }
        else
        {
            clipboard.last->next = newLine;
            newLine->prev = clipboard.last;
            clipboard.last = newLine;
        }

        return newLine;
    }

    void addClipboardChar(LineNode* line, char ch)
    {
        CharNode* newNode = new CharNode(ch);

        if (line->head == NULL)
        {
            line->head = newNode;
            line->tail = newNode;
        }
        else
        {
            newNode->prev = line->tail;
            line->tail->next = newNode;
            line->tail = newNode;
        }
    }

    bool hasSelection()
    {
        return selecting &&
               !samePosition(selectionStart, selectionEnd);
    }

    void beginSelection()
    {
        if (!selecting)
        {
            selectionStart = getCursorPosition();
            selectionEnd = selectionStart;
            selecting = true;
        }
    }

    void updateSelection()
    {
        if (selecting)
            selectionEnd = getCursorPosition();
    }

    void clearSelection()
    {
        selecting = false;
        selectionStart = Position();
        selectionEnd = Position();
    }

    bool isCharacterSelected(LineNode* line, CharNode* ch)
    {
        if (!hasSelection())
            return false;

        Position start = selectionStart;
        Position end = selectionEnd;
        normalizeSelection(start, end);

        int lineNo = getLineNumber(line);
        int startLineNo = getLineNumber(start.line);
        int endLineNo = getLineNumber(end.line);

        if (lineNo < startLineNo || lineNo > endLineNo)
            return false;

        int col = getColumn(line, ch);
        int lineLength = getLineLength(line);

        int startCol = 0;
        int endCol = lineLength;

        if (line == start.line)
            startCol = getColumn(start.line, start.cursor);

        if (line == end.line)
            endCol = getColumn(end.line, end.cursor);

        return col >= startCol && col < endCol;
    }

    void copySelection()
    {
        if (!hasSelection())
            return;

        Position start = selectionStart;
        Position end = selectionEnd;
        normalizeSelection(start, end);

        clearClipboard();

        LineNode* clipLine = addClipboardLine();
        Position pos = start;

        while (!samePosition(pos, end))
        {
            if (pos.cursor == NULL)
            {
                if (pos.line->next != NULL)
                {
                    pos.line = pos.line->next;
                    pos.cursor = pos.line->head;
                    clipLine = addClipboardLine();
                }
                else
                {
                    break;
                }

                continue;
            }

            addClipboardChar(clipLine, pos.cursor->data);

            bool crossedLine =
                (pos.cursor->next == NULL &&
                 pos.line->next != NULL);

            movePositionForward(pos);

            if (crossedLine)
            {
                clipLine = addClipboardLine();
            }
        }
    }

    void deleteSelection()
    {
        if (!hasSelection())
            return;

        Position start = selectionStart;
        Position end = selectionEnd;
        normalizeSelection(start, end);

        // Same-line selection.
        if (start.line == end.line)
        {
            CharNode* ch = start.cursor;

            while (ch != end.cursor)
            {
                if (ch == NULL)
                    break;

                CharNode* next = ch->next;
                CharNode* left = ch->prev;

                if (left != NULL)
                    left->next = next;
                else
                    start.line->head = next;

                if (next != NULL)
                    next->prev = left;
                else
                    start.line->tail = left;

                delete ch;
                ch = next;
            }

            currentLine = start.line;
            currentLineNumber = getLineNumber(currentLine);
            cursor = end.cursor;

            clearSelection();
            rebuildSkipList();
            return;
        }

        // Keep the part before selection from the first line.
        CharNode* leftTail = NULL;
        if (start.cursor != NULL)
            leftTail = start.cursor->prev;
        else
            leftTail = start.line->tail;

        // Delete from start position to the end of the first line.
        CharNode* ch = start.cursor;
        while (ch != NULL)
        {
            CharNode* next = ch->next;
            delete ch;
            ch = next;
        }

        if (leftTail != NULL)
        {
            leftTail->next = NULL;
            start.line->tail = leftTail;
        }
        else
        {
            start.line->head = NULL;
            start.line->tail = NULL;
        }

        // Delete all complete lines between start and end.
        LineNode* line = start.line->next;
        while (line != end.line)
        {
            LineNode* nextLine = line->next;

            CharNode* c = line->head;
            while (c != NULL)
            {
                CharNode* nextChar = c->next;
                delete c;
                c = nextChar;
            }

            delete line;
            line = nextLine;
        }

        // Delete the part of the final line before end.cursor.
        CharNode* endCursor = end.cursor;
        CharNode* suffix = endCursor;

        // Append the suffix of the end line to the first line.
        if (suffix != NULL)
        {
            CharNode* suffixPrev = suffix->prev;

            while (suffixPrev != NULL)
            {
                CharNode* prev = suffixPrev->prev;
                delete suffixPrev;
                suffixPrev = prev;
            }
        }
        else
        {
            CharNode* c = end.line->head;
            while (c != NULL)
            {
                CharNode* next = c->next;
                delete c;
                c = next;
            }
        }

        // Connect the surviving suffix to the first line.
        if (suffix != NULL)
        {
            if (leftTail != NULL)
            {
                leftTail->next = suffix;
                suffix->prev = leftTail;
            }
            else
            {
                start.line->head = suffix;
                suffix->prev = NULL;
            }

            start.line->tail = end.line->tail;
        }
        else
        {
            if (leftTail != NULL)
                start.line->tail = leftTail;
            else
                start.line->head = start.line->tail = NULL;
        }

        // Remove end.line from the line list.
        LineNode* after = end.line->next;
        start.line->next = after;

        if (after != NULL)
            after->prev = start.line;
        else
            lastLine = start.line;

        if (end.line != start.line)
            delete end.line;

        currentLine = start.line;
        currentLineNumber = getLineNumber(currentLine);
        cursor = suffix;

        clearSelection();
        rebuildSkipList();
    }

    void cutSelection()
    {
        if (!hasSelection())
            return;

        copySelection();
        deleteSelection();
    }

    void pasteClipboard()
    {
        if (clipboard.first == NULL)
            return;

        LineNode* clipLine = clipboard.first;

        while (clipLine != NULL)
        {
            CharNode* ch = clipLine->head;

            while (ch != NULL)
            {
                insert(ch->data);
                ch = ch->next;
            }

            if (clipLine->next != NULL)
                newLine();

            clipLine = clipLine->next;
        }

        clearSelection();
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

        selecting = false;
        selectionStart = Position();
        selectionEnd = Position();

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

        HANDLE hOutput =
            GetStdHandle(STD_OUTPUT_HANDLE);

        LineNode* line = firstLine;

        while (line != NULL)
        {
            CharNode* temp = line->head;

            while (temp != NULL)
            {
                if (isCharacterSelected(line, temp))
                {
                    SetConsoleTextAttribute(hOutput, 240);
                }
                else
                {
                    SetConsoleTextAttribute(hOutput, 7);
                }

                cout << temp->data;
                temp = temp->next;
            }

            SetConsoleTextAttribute(hOutput, 7);
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

            bool ctrlPressed =
                (key.dwControlKeyState &
                 (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;

            bool shiftPressed =
                (key.dwControlKeyState & SHIFT_PRESSED) != 0;

            if (key.wVirtualKeyCode == VK_ESCAPE)
            {
                if (selecting)
                    clearSelection();
                else
                    break;
            }
            else if (ctrlPressed && key.wVirtualKeyCode == 'C')
            {
                copySelection();
            }
            else if (ctrlPressed && key.wVirtualKeyCode == 'X')
            {
                cutSelection();
            }
            else if (ctrlPressed && key.wVirtualKeyCode == 'V')
            {
                pasteClipboard();
            }
            else if (shiftPressed &&
                     (key.wVirtualKeyCode == VK_LEFT ||
                      key.wVirtualKeyCode == VK_RIGHT ||
                      key.wVirtualKeyCode == VK_UP ||
                      key.wVirtualKeyCode == VK_DOWN))
            {
                beginSelection();

                if (key.wVirtualKeyCode == VK_LEFT)
                    moveLeft();
                else if (key.wVirtualKeyCode == VK_RIGHT)
                    moveRight();
                else if (key.wVirtualKeyCode == VK_UP)
                    moveUp();
                else if (key.wVirtualKeyCode == VK_DOWN)
                    moveDown();

                updateSelection();
            }
            else if (key.wVirtualKeyCode == VK_LEFT)
            {
                clearSelection();
                moveLeft();
            }
            else if (key.wVirtualKeyCode == VK_RIGHT)
            {
                clearSelection();
                moveRight();
            }
            else if (key.wVirtualKeyCode == VK_UP)
            {
                clearSelection();
                moveUp();
            }
            else if (key.wVirtualKeyCode == VK_DOWN)
            {
                clearSelection();
                moveDown();
            }
            else if (key.wVirtualKeyCode == VK_BACK)
            {
                if (hasSelection())
                    deleteSelection();
                else
                    backspace();
            }
            else if (key.wVirtualKeyCode == VK_DELETE)
            {
                if (hasSelection())
                    deleteSelection();
                else
                    deleteChar();
            }
            else if (key.wVirtualKeyCode == VK_RETURN)
            {
                clearSelection();
                newLine();
            }
            else if (key.uChar.AsciiChar >= 32 &&
                     key.uChar.AsciiChar <= 126)
            {
                clearSelection();
                insert(key.uChar.AsciiChar);
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
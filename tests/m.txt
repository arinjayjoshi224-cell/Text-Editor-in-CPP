#include <iostream>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <climits>
#include <cstring>
#include "FileHandler.h"

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

// Swallow Ctrl+C so it can be used as "copy" instead of killing the program.
BOOL WINAPI ctrlHandler(DWORD type)
{
    return type == CTRL_C_EVENT;
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
    string currentFilename;
    LineNode* firstLine;
    LineNode* lastLine;
    LineNode* currentLine;

    CharNode* cursor;

    int currentLineNumber;

    SkipNode* skipHead;
    int skipLevel;

    // ---- selection / clipboard state ----
    bool hasSelection;      // true while a selection anchor is active
    int anchorLine;         // where the selection started
    int anchorCol;
    string clipboard;       // raw text of last copy/cut ('\n' separates lines)

    // ---- clipboard as a linked list ----
    // One LineNode per clipboard line, each holding a CharNode chain.
    // Built once on copy/cut; on paste the chains are cloned and spliced
    // straight into the text instead of inserting char-by-char.
    LineNode* clipHead;
    LineNode* clipTail;
    int clipLineCount;

    void clearClipList()
    {
        LineNode* line = clipHead;

        while (line != NULL)
        {
            CharNode* c = line->head;

            while (c != NULL)
            {
                CharNode* nextChar = c->next;
                delete c;
                c = nextChar;
            }

            LineNode* nextLine = line->next;
            delete line;
            line = nextLine;
        }

        clipHead = NULL;
        clipTail = NULL;
        clipLineCount = 0;
    }

    void appendChar(LineNode* line, char ch)
    {
        CharNode* n = new CharNode(ch);

        if (line->tail == NULL)
        {
            line->head = n;
            line->tail = n;
        }
        else
        {
            n->prev = line->tail;
            line->tail->next = n;
            line->tail = n;
        }
    }

    // Turn text into the clipboard linked list (one pass).
    void buildClipList(const string& text)
    {
        clearClipList();

        LineNode* line = new LineNode();

        clipHead = line;
        clipTail = line;
        clipLineCount = 1;

        for (size_t i = 0; i < text.size(); i++)
        {
            char ch = text[i];

            if (ch == '\n')
            {
                LineNode* nl = new LineNode();

                nl->prev = clipTail;
                clipTail->next = nl;
                clipTail = nl;
                line = nl;

                clipLineCount++;
            }
            else if (ch == '\t')
            {
                for (int k = 0; k < 4; k++)
                    appendChar(line, ' ');
            }
            else if (ch >= 32 && ch <= 126)
            {
                appendChar(line, ch);
            }
        }
    }

    // Deep-copies a CharNode chain (the clipboard must survive many pastes).
    void cloneChain(CharNode* src, CharNode*& h, CharNode*& t)
    {
        h = NULL;
        t = NULL;

        for (; src != NULL; src = src->next)
        {
            CharNode* n = new CharNode(src->data);

            if (t == NULL)
            {
                h = n;
                t = n;
            }
            else
            {
                n->prev = t;
                t->next = n;
                t = n;
            }
        }
    }

    // Splice one chain h..t into the current line at the cursor.
    // The cursor stays on the character that followed the cursor before.
    void insertChainAtCursor(CharNode* h, CharNode* t)
    {
        if (h == NULL)
            return;

        if (currentLine->head == NULL)
        {
            currentLine->head = h;
            currentLine->tail = t;
            cursor = NULL;
            return;
        }

        if (cursor == NULL)
        {
            h->prev = currentLine->tail;
            currentLine->tail->next = h;
            currentLine->tail = t;
            return;
        }

        CharNode* left = cursor->prev;

        h->prev = left;
        t->next = cursor;
        cursor->prev = t;

        if (left != NULL)
            left->next = h;
        else
            currentLine->head = h;
    }

    // Splice the whole clipboard list into the text at the cursor.
    // The skip list is rebuilt once, not once per pasted line.
    void pasteClipList()
    {
        if (clipHead == NULL)
            return;

        CharNode* h;
        CharNode* t;

        // ---- single-line clipboard: one chain splice ----
        if (clipLineCount == 1)
        {
            cloneChain(clipHead->head, h, t);
            insertChainAtCursor(h, t);
            return;
        }

        // ---- multi-line clipboard ----
        LineNode* cur = currentLine;

        // 1. Cut the current line at the cursor; keep the right half aside.
        CharNode* rightHead = cursor;
        CharNode* rightTail = (cursor != NULL) ? cur->tail : NULL;

        if (cursor != NULL)
        {
            CharNode* left = cursor->prev;

            if (left != NULL)
            {
                left->next = NULL;
                cur->tail = left;
            }
            else
            {
                cur->head = NULL;
                cur->tail = NULL;
            }

            cursor->prev = NULL;
        }

        // 2. First clipboard line continues the current line.
        cloneChain(clipHead->head, h, t);

        if (h != NULL)
        {
            if (cur->tail != NULL)
            {
                cur->tail->next = h;
                h->prev = cur->tail;
                cur->tail = t;
            }
            else
            {
                cur->head = h;
                cur->tail = t;
            }
        }

        // 3. Remaining clipboard lines become new lines after it.
        LineNode* after = cur->next;
        LineNode* last = cur;

        for (LineNode* src = clipHead->next; src != NULL; src = src->next)
        {
            LineNode* nl = new LineNode();

            cloneChain(src->head, nl->head, nl->tail);

            nl->prev = last;
            last->next = nl;
            last = nl;
        }

        // 4. Re-attach the right half after the last pasted line.
        if (rightHead != NULL)
        {
            if (last->tail != NULL)
            {
                last->tail->next = rightHead;
                rightHead->prev = last->tail;
                last->tail = rightTail;
            }
            else
            {
                last->head = rightHead;
                last->tail = rightTail;
            }
        }

        last->next = after;

        if (after != NULL)
            after->prev = last;
        else
            lastLine = last;

        currentLine = last;
        cursor = rightHead;
        currentLineNumber += clipLineCount - 1;

        rebuildSkipList();
    }

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
        currentFilename = "";

        firstLine = new LineNode();

        lastLine = firstLine;
        currentLine = firstLine;

        cursor = NULL;

        currentLineNumber = 0;

        skipHead = NULL;
        skipLevel = 1;

        hasSelection = false;
        anchorLine = 0;
        anchorCol = 0;

        clipHead = NULL;
        clipTail = NULL;
        clipLineCount = 0;

        rebuildSkipList();
    }

    ~TextEditor()
    {
        clearClipList();
    }

    // Add .txt if the user did not specify an extension.
    string normalizeFilename(string filename)
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

    // Save to the current filename, or ask for a new one.
    bool saveDocument(bool saveAs = false)
    {
        if (saveAs || currentFilename.empty())
        {
            cout << "\nEnter filename or full path: ";

            string filename;
            getline(cin, filename);

            if (filename.empty())
            {
                cout << "Save cancelled.\n";
                return false;
            }

            currentFilename = normalizeFilename(filename);
        }

        if (saveToFile(currentFilename, getText()))
        {
            cout << "\nSaved successfully: "
                 << currentFilename << "\n";
            return true;
        }

        cout << "\nCould not save file: "
             << currentFilename << "\n";
        return false;
    }

    // Open a specific file selected by its path.
    bool openDocument()
    {
        cout << "\nEnter filename or full path to open: ";

        string filename;
        getline(cin, filename);

        if (filename.empty())
        {
            cout << "Open cancelled.\n";
            return false;
        }

        filename = normalizeFilename(filename);

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

    // Convert the editor's linked-list text into a string.
    string getText()
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

    // Replace the editor's contents with loaded text.
    void setText(const string& content)
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

    // =====================================================
    //  Selection helpers
    // =====================================================

    // Move the cursor to an absolute (line, column) position.
    void moveTo(int line, int column)
    {
        LineNode* l = firstLine;
        int n = 0;

        while (n < line && l->next != NULL)
        {
            l = l->next;
            n++;
        }

        currentLine = l;
        currentLineNumber = n;

        setCursorColumn(column);
    }

    // Returns the selection as an ordered range [start, end).
    // Returns false if nothing is selected.
    bool getSelectionRange(int& sl, int& sc, int& el, int& ec)
    {
        if (!hasSelection)
            return false;

        int curLine = currentLineNumber;
        int curCol = getCurrentColumn();

        if (curLine == anchorLine && curCol == anchorCol)
            return false;

        if (anchorLine < curLine ||
            (anchorLine == curLine && anchorCol < curCol))
        {
            sl = anchorLine; sc = anchorCol;
            el = curLine;    ec = curCol;
        }
        else
        {
            sl = curLine;    sc = curCol;
            el = anchorLine; ec = anchorCol;
        }

        return true;
    }

    bool selectionActive()
    {
        int a, b, c, d;
        return getSelectionRange(a, b, c, d);
    }

    void beginSelection()
    {
        if (!hasSelection)
        {
            hasSelection = true;
            anchorLine = currentLineNumber;
            anchorCol = getCurrentColumn();
        }
    }

    void clearSelection()
    {
        hasSelection = false;
    }

    void selectAll()
    {
        hasSelection = true;
        anchorLine = 0;
        anchorCol = 0;

        moveTo(INT_MAX, INT_MAX);   // clamps to last line, end of line
    }

    // Text between two positions; lines are joined with '\n'.
    string textBetween(int sl, int sc, int el, int ec)
    {
        string result;

        LineNode* line = firstLine;
        int n = 0;

        while (line != NULL && n < sl)
        {
            line = line->next;
            n++;
        }

        for (; line != NULL && n <= el; n++, line = line->next)
        {
            int from = (n == sl) ? sc : 0;
            int to = (n == el) ? ec : INT_MAX;

            int col = 0;

            for (CharNode* c = line->head; c != NULL; c = c->next, col++)
            {
                if (col >= to)
                    break;

                if (col >= from)
                    result += c->data;
            }

            if (n < el)
                result += '\n';
        }

        return result;
    }

    // Deletes the selected text and leaves the cursor where it began.
    // Returns true if something was deleted.
    bool deleteSelection()
    {
        int sl, sc, el, ec;

        if (!getSelectionRange(sl, sc, el, ec))
        {
            clearSelection();
            return false;
        }

        int count = (int)textBetween(sl, sc, el, ec).size();

        moveTo(sl, sc);

        // deleteChar() at end-of-line joins the next line,
        // so every character and every '\n' costs one call.
        for (int i = 0; i < count; i++)
            deleteChar();

        clearSelection();

        return true;
    }

    // =====================================================
    //  System clipboard (Windows) – lets you paste to/from
    //  other applications. Falls back to the internal one.
    // =====================================================

    void setSystemClipboard(const string& text)
    {
        string out;

        for (size_t i = 0; i < text.size(); i++)
        {
            if (text[i] == '\n')
                out += "\r\n";
            else
                out += text[i];
        }

        if (!OpenClipboard(NULL))
            return;

        EmptyClipboard();

        HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, out.size() + 1);

        if (mem != NULL)
        {
            char* dst = (char*)GlobalLock(mem);

            memcpy(dst, out.c_str(), out.size() + 1);

            GlobalUnlock(mem);

            if (SetClipboardData(CF_TEXT, mem) == NULL)
                GlobalFree(mem);
        }

        CloseClipboard();
    }

    bool getSystemClipboard(string& text)
    {
        if (!OpenClipboard(NULL))
            return false;

        HANDLE h = GetClipboardData(CF_TEXT);

        if (h == NULL)
        {
            CloseClipboard();
            return false;
        }

        const char* src = (const char*)GlobalLock(h);

        if (src == NULL)
        {
            CloseClipboard();
            return false;
        }

        text.clear();

        for (; *src; src++)
        {
            if (*src == '\r')
                continue;               // "\r\n" -> "\n"

            text += *src;
        }

        GlobalUnlock(h);
        CloseClipboard();

        return true;
    }

    // =====================================================
    //  Copy / Cut / Paste
    // =====================================================

    void copy()
    {
        int sl, sc, el, ec;

        if (!getSelectionRange(sl, sc, el, ec))
            return;

        clipboard = textBetween(sl, sc, el, ec);

        buildClipList(clipboard);           // text -> linked list

        setSystemClipboard(clipboard);
    }

    void cut()
    {
        if (!selectionActive())
            return;

        copy();
        deleteSelection();
    }

    void paste()
    {
        string text;

        bool haveSystem = getSystemClipboard(text) && !text.empty();

        // Reuse the prebuilt list; only rebuild if another app changed
        // the system clipboard since our last copy/cut.
        if (haveSystem && text != clipboard)
        {
            clipboard = text;
            buildClipList(text);
        }

        if (clipHead == NULL ||
            (clipLineCount == 1 && clipHead->head == NULL))
            return;

        // Pasting over a selection replaces it
        deleteSelection();

        pasteClipList();
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

        // Cursor ends up at the join point: just before the first
        // character that came from the next line (NULL if none).
        CharNode* joinPoint = nextLine->head;

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

        cursor = joinPoint;

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

    void flushBuffer(HANDLE hOut, string& buf)
    {
        if (buf.empty())
            return;

        DWORD written;

        WriteConsoleA(hOut, buf.c_str(), (DWORD)buf.size(), &written, NULL);

        buf.clear();
    }

    // Fast replacement for system("cls") (which spawns a process).
    void clearScreen(HANDLE hOut)
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

    void display()
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

        clearScreen(hOut);

        const WORD normalAttr =
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        const WORD selectedAttr =
            BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;

        int sl = 0, sc = 0, el = 0, ec = 0;
        bool selecting = getSelectionRange(sl, sc, el, ec);

        // Characters are collected into a buffer and written in one call
        // per run of identical colour, instead of one cout per character.
        string buf;
        WORD curAttr = normalAttr;

        SetConsoleTextAttribute(hOut, curAttr);

        LineNode* line =
            firstLine;

        int row = 0;

        while (line != NULL)
        {
            CharNode* temp =
                line->head;

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

        // No ENABLE_PROCESSED_INPUT: Ctrl+C arrives as a normal key event.
        SetConsoleMode(
            hInput,
            ENABLE_EXTENDED_FLAGS
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
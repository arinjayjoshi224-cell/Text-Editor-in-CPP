#include "TextEditor.h"
#include "FileHandler.h"
#include "ConsoleHelpers.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

void TextEditor::clearClipList()
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

void TextEditor::appendChar(LineNode* line, char ch)
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

void TextEditor::buildClipList(const string& text)
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

void TextEditor::cloneChain(CharNode* src, CharNode*& h, CharNode*& t)
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

void TextEditor::insertChainAtCursor(CharNode* h, CharNode* t)
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

void TextEditor::pasteClipList()
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

bool TextEditor::getSelectionRange(int& sl, int& sc, int& el, int& ec)
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

void TextEditor::changeSelectionColor(unsigned short color)
{
    int sl, sc, el, ec;

    if (!getSelectionRange(sl, sc, el, ec))
        return;

    EditorSnapshot before = captureSnapshot();

    LineNode* line = getLine(sl);

    for (int row = sl; row <= el && line != NULL; row++)
    {
        CharNode* ch = line->head;
        int col = 0;

        while (ch != NULL)
        {
            if ((row > sl || col >= sc) &&
                (row < el || col < ec))
            {
                ch->color = color;
            }

            ch = ch->next;
            col++;
        }

        line = line->next;
    }

    typingGroupOpen = false;
    recordEdit(before, false);
}

bool TextEditor::selectionActive()
{
    int a, b, c, d;
    return getSelectionRange(a, b, c, d);
}

void TextEditor::beginSelection()
{
    if (!hasSelection)
    {
        hasSelection = true;
        anchorLine = currentLineNumber;
        anchorCol = getCurrentColumn();
    }
}

void TextEditor::clearSelection()
{
    hasSelection = false;
}

void TextEditor::selectAll()
{
    hasSelection = true;
    anchorLine = 0;
    anchorCol = 0;

    moveTo(INT_MAX, INT_MAX);   // clamps to last line, end of line
}

string TextEditor::textBetween(int sl, int sc, int el, int ec)
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

bool TextEditor::deleteSelection()
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

void TextEditor::setSystemClipboard(const string& text)
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

bool TextEditor::getSystemClipboard(string& text)
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

void TextEditor::copy()
{
    int sl, sc, el, ec;

    if (!getSelectionRange(sl, sc, el, ec))
        return;

    clipboard = textBetween(sl, sc, el, ec);

    buildClipList(clipboard);           // text -> linked list

    setSystemClipboard(clipboard);
}

void TextEditor::cut()
{
    if (!selectionActive())
        return;

    copy();
    deleteSelection();
}

void TextEditor::paste()
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


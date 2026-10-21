#include "TextEditor.h"
#include <cstdlib>
#include <ctime>

TextEditor::TextEditor()
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

TextEditor::~TextEditor()
{
        clearClipList();
    }

void TextEditor::insert(char ch)
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

void TextEditor::backspace()
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

void TextEditor::deleteChar()
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

void TextEditor::mergeWithPreviousLine()
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

void TextEditor::mergeWithNextLine()
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

void TextEditor::newLine()
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


#include "TextEditor.h"
#include "FileHandler.h"
#include "ConsoleHelpers.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

int TextEditor::getCurrentColumn()
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

int TextEditor::getLineLength(LineNode* line)
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

void TextEditor::setCursorColumn(int column)
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

void TextEditor::updateCurrentLineNumber()
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

void TextEditor::moveTo(int line, int column)
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

void TextEditor::moveLeft()
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

void TextEditor::moveRight()
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

void TextEditor::moveUp()
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

void TextEditor::moveDown()
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


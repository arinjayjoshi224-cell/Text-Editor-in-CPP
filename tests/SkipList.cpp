#include "TextEditor.h"
#include "FileHandler.h"
#include "ConsoleHelpers.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

void TextEditor::clearSkipList()
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

void TextEditor::rebuildSkipList()
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

LineNode* TextEditor::getLine(int lineNumber)
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
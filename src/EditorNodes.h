#ifndef EDITORNODES_H
#define EDITORNODES_H
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


#endif

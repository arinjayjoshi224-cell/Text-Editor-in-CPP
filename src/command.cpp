#include <iostream>
#include <string>
#include <stack>
using namespace std;

class Command
{
    struct Node
    {
        char data;
        Node* prev;
        Node* next;

        Node(char ch)
        {
            data = ch;
            prev = NULL;
            next = NULL;
        }
    };

    struct Operation
    {
        string type;
        int pos;
        string text;

        Operation(string t, int p, string txt)
        {
            type = t;
            pos = p;
            text = txt;
        }
    };

    Node* head = NULL;

    stack<Operation> undoStack;
    stack<Operation> redoStack;

public:

    void copy()
    {
    }

    void cut()
    {
    }

    void paste()
    {
    }

    void undo()
    {
    }

    void redo()
    {
    }
};

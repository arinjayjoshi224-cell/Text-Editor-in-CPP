#include <iostream>
#include <windows.h>
using namespace std;

struct Node
{
    char data;
    Node *prev;
    Node *next;

    Node(char ch)
    {
        data = ch;
        prev = NULL;
        next = NULL;
    }
};

class TextEditor
{
private:
    Node *head;
    Node *tail;
    Node *cursor;

public:

    TextEditor()
    {
        head = NULL;
        tail = NULL;
        cursor = NULL;
    }

    void insert(char ch)
    {
        Node *newNode = new Node(ch);

       
        if (head == NULL)
        {
            head = tail = newNode;
            cursor = NULL;
            return;
        }

      
        if (cursor == NULL)
        {
            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;
            return;
        }

        
        Node *left = cursor->prev;

        newNode->next = cursor;
        newNode->prev = left;

        cursor->prev = newNode;

        if (left != NULL)
            left->next = newNode;
        else
            head = newNode;
    }

   
    void moveLeft()
    {
        if (cursor == NULL)
        {
            if (tail != NULL)
                cursor = tail;
        }
        else if (cursor->prev != NULL)
        {
            cursor = cursor->prev;
        }
    }

  
    void moveRight()
    {
        if (cursor != NULL)
            cursor = cursor->next;
    }

    void backspace()
    {
        if (head == NULL)
            return;

        Node *del;

     
        if (cursor == NULL)
        {
            del = tail;
            tail = tail->prev;

            if (tail != NULL)
                tail->next = NULL;
            else
                head = NULL;

            delete del;
            return;
        }

       
        if (cursor->prev != NULL)
        {
            del = cursor->prev;
            Node *left = del->prev;

            cursor->prev = left;

            if (left != NULL)
                left->next = cursor;
            else
                head = cursor;

            delete del;
        }
    }

   
    void deleteChar()
    {
        if (cursor == NULL)
            return;

        Node *del = cursor;
        cursor = cursor->next;

        if (del->prev != NULL)
            del->prev->next = cursor;
        else
            head = cursor;

        if (cursor != NULL)
            cursor->prev = del->prev;
        else
            tail = del->prev;

        delete del;
    }

   
    void display()
    {
        //system("cls");

        Node *temp = head;

        while (temp != NULL)
        {
            if (temp == cursor)
                cout << "|";

            cout << temp->data;

            temp = temp->next;
        }

      
        if (cursor == NULL)
            cout << "|";

        cout << "\n\n";

    }

  
    void start()
    {
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

        DWORD mode;
        GetConsoleMode(hInput, &mode);

       
        SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS);

        INPUT_RECORD input;
        DWORD events;

        while (true)
        {
            display();

            ReadConsoleInput(hInput, &input, 1, &events);

            if (input.EventType != KEY_EVENT)
                continue;

            KEY_EVENT_RECORD key = input.Event.KeyEvent;

            if (!key.bKeyDown)
                continue;

            // ESC
            if (key.wVirtualKeyCode == VK_ESCAPE)
                break;

            // Left arrow
            else if (key.wVirtualKeyCode == VK_LEFT)
                moveLeft();

            // Right arrow
            else if (key.wVirtualKeyCode == VK_RIGHT)
                moveRight();

            // Backspace
            else if (key.wVirtualKeyCode == VK_BACK)
                backspace();

            // Delete
            else if (key.wVirtualKeyCode == VK_DELETE)
                deleteChar();

            // Normal character
            else if (key.uChar.AsciiChar >= 32 &&
                     key.uChar.AsciiChar <= 126)
            {
                insert(key.uChar.AsciiChar);
            }
        }
    }

   
    void showList()
    {
        cout << "\nCharacters stored in Doubly Linked List:\n";

        Node *temp = head;

        while (temp != NULL)
        {
            cout << "[" << temp->data << "]";

            if (temp->next != NULL)
                cout << " <-> ";

            temp = temp->next;
        }

        cout << endl;
    }
};

int main()
{
    TextEditor editor;

    editor.start();
    system("cls");
    cout << "Final Text: ";
    editor.showList();

    return 0;
}
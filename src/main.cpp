#include "TextEditor.h"
#include <iostream>
#include <cstdlib>
using namespace std;

int main()
{
    TextEditor editor;

    editor.start();

    system("cls");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    cout << "Final Text:\n\n";
    editor.showList();

    return 0;
}
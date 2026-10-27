#include "TextEditor.h"
#include <iostream>
#include <cstdlib>
using namespace std;

int main()
{
    TextEditor editor;

    editor.start();

    system("cls");

    cout << "Final Text:\n\n";
    editor.showList();

    return 0;
}
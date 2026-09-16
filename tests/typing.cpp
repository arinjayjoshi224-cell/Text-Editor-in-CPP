#include <iostream>
#include <conio.h>
using namespace std;

int main()
{
    char ch;
    cout << "Start typing (Press ESC to stop):\n";
    while (1)
    {
        ch = getch();
        if (ch == 27) 
            break;
        cout << ch << endl;
    }
    return 0;
}
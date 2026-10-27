@echo off

echo Compiling Text Editor...

g++ -std=c++17 main.cpp command.cpp TextEditor.cpp Clipboard.cpp Cursor.cpp SkipList.cpp Document.cpp Display.cpp InputHandler.cpp ConsoleHelpers.cpp FileHandler.cpp -o editor.exe

if errorlevel 1 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo Compilation successful!

editor.exe
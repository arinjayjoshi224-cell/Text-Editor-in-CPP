
@echo off
cd /d "%~dp0\.."

echo Compiling Text Editor...

g++ -std=c++17 src/main.cpp src/command.cpp src/TextEditor.cpp src/Clipboard.cpp src/Cursor.cpp src/SkipList.cpp src/Document.cpp src/Display.cpp src/InputHandler.cpp src/ConsoleHelpers.cpp src/FileHandler.cpp -o src/editor.exe -lcomdlg32

if errorlevel 1 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo Compilation successful!
src\editor.exe
pause

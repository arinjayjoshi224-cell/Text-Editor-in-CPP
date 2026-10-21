# Build (Windows MinGW)

From this directory:

```powershell
g++ -std=c++17 typing.cpp TextEditor.cpp Clipboard.cpp Cursor.cpp SkipList.cpp Document.cpp Display.cpp InputHandler.cpp ConsoleHelpers.cpp FileHandler.cpp -o editor.exe
.\editor.exe
```

Compile and link all listed .cpp files. command.cpp and customWindow.svg remain unchanged and are excluded from this build.

- typing.cpp: main()
- TextEditor.h: shared class declarations
- EditorNodes.h: character, line and skip nodes
- TextEditor.cpp: core editing and line splitting/merging
- Clipboard.cpp: selection, cut/copy/paste and system clipboard
- Cursor.cpp: cursor movement and positioning
- SkipList.cpp: skip-list lookup
- Document.cpp: save/open, text serialization
- Display.cpp: console rendering
- InputHandler.cpp: keyboard shortcuts
- ConsoleHelpers.h/.cpp: console utilities
- FileHandler.h/.cpp: existing file IO, unchanged

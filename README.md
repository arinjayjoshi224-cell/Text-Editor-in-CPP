# C++ Console Text Editor

A console-based text editor developed in **C++17** using custom data structures and a modular, object-oriented design. The project focuses on implementing text editing, cursor navigation, clipboard operations, and file handling without relying on a built-in text editor widget.

**Repository:** [Text-Editor-in-CPP](https://github.com/arinjayjoshi224-cell/Text-Editor-in-CPP)

## Features

- **Text Editing:** Insert and delete characters while editing text.
- **Multiline Editing:** Create new lines and merge existing lines.
- **Cursor Navigation:** Move the cursor through the document using keyboard controls.
- **Text Selection:** Select text for clipboard operations.
- **Clipboard Operations:** Copy, cut, and paste text, including integration with the Windows system clipboard.
- **File Handling:** Open and save text files.
- **Efficient Line Lookup:** Use a skip list to support line navigation.
- **Modular Architecture:** Separate editing, cursor handling, clipboard management, document operations, display, and input processing into individual source files.

## Technology Stack

| Component | Technology |
|---|---|
| Programming language | C++17 |
| Compiler | MinGW `g++` |
| Development environment | Visual Studio Code |
| Platform | Windows |
| Console interface | Windows Console API |
| Version control | Git and GitHub |

## Data Structures Used

### 1. Doubly Linked List of Characters

Each line contains a doubly linked list of character nodes. This structure supports character-level editing and movement through text.

### 2. Doubly Linked List of Lines

The document stores its lines in a doubly linked list. Each line maintains its own character list, allowing the editor to manage multiline text and perform line insertion or merging.

### 3. Skip List

A skip list provides an additional index for locating lines, supporting more efficient line lookup than traversing the entire line list for every lookup.

## Project Structure

| File | Responsibility |
|---|---|
| `typing.cpp` | Program entry point (`main()`) |
| `TextEditor.h` | Shared editor class declarations |
| `TextEditor.cpp` | Core editing operations and line splitting/merging |
| `EditorNodes.h` | Character, line, and skip-list node definitions |
| `Clipboard.cpp` | Text selection, copy, cut, paste, and system clipboard integration |
| `Cursor.cpp` | Cursor movement and positioning |
| `SkipList.cpp` | Skip-list operations and line lookup |
| `Document.cpp` | Document serialization and open/save operations |
| `Display.cpp` | Console rendering |
| `InputHandler.cpp` | Keyboard input and shortcut handling |
| `ConsoleHelpers.h/.cpp` | Console utility functions |
| `FileHandler.h/.cpp` | File I/O utilities |
| `build.bat` | Optional Windows build automation |
| `README.md` | Project documentation |

`command.cpp` and `customWindow.svg` are currently excluded from the build described below.

## Requirements

Before building the project, ensure you have:

- Windows
- MinGW-w64 with `g++` supporting C++17
- A terminal where the compiler is available through `PATH`

## Build and Run

### Option 1: Build manually

Open a terminal in the project directory and execute:

```powershell
g++ -std=c++17 typing.cpp TextEditor.cpp Clipboard.cpp Cursor.cpp SkipList.cpp Document.cpp Display.cpp InputHandler.cpp ConsoleHelpers.cpp FileHandler.cpp -o editor.exe
```

After compilation succeeds, run:

```powershell
.\editor.exe
```

Compile all listed `.cpp` files together because they implement different parts of the editor.

### Option 2: Use the build script

If `build.bat` is present and contains the same compilation command, run it from the project directory:

```powershell
.\build.bat
```

The script can automate compilation and, if configured to do so, launch the resulting executable.

## Architecture

The editor separates input processing, document editing, cursor navigation, clipboard management, rendering, and file operations into dedicated modules.

```text
Keyboard Input
      |
      v
InputHandler
      |
      v
TextEditor Core
      |
      +------ Cursor
      |
      +------ Clipboard
      |
      +------ Document / FileHandler
      |
      +------ SkipList
      |
      v
Display / Console
```

The document is represented using nested doubly linked lists, while the skip list supports line lookup. The separate modules interact through the shared editor declarations.

## Current Development Status

The project currently focuses on a working console-based editor and its core editing functionality.

**Implemented areas**
- Character and multiline editing
- Cursor navigation
- Text selection and clipboard operations
- File handling and console display
- Modular source-code organization

**Future improvements**
- Develop a graphical user interface (GUI).
- Implement undo and redo functionality.
- Expand testing for cursor navigation, multiline editing, clipboard operations, and file handling.
- Improve the user interface and overall editing experience.

Features should be considered verified only after testing the current modular build.

## Learning Objectives

This project provides practical experience with:

- Object-oriented programming in C++
- Pointers and dynamic memory management
- Doubly linked lists and skip lists
- Modular software design
- File handling and operating-system clipboard integration
- Compilation, debugging, and version control

## Contributors

MEHULLI DASTIDAR
MAHEK DATTA
ARINJAY JOSHI
ARUNABHA MANDAL

## License

This project is licensed under the MIT License.
See the [LICENSE](LICENSE) file for details.

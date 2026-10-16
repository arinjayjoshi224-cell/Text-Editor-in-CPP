#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include "EditorNodes.h"

using namespace std;

class TextEditor
{
private:
    string currentFilename;
    LineNode* firstLine;
    LineNode* lastLine;
    LineNode* currentLine;
    CharNode* cursor;
    int currentLineNumber;
    SkipNode* skipHead;
    int skipLevel;
    bool hasSelection;      // true while a selection anchor is active
    int anchorLine;         // where the selection started
    int anchorCol;
    string clipboard;       // raw text of last copy/cut ('\n' separates lines)
    LineNode* clipHead;
    LineNode* clipTail;
    int clipLineCount;

    void clearClipList();
    void appendChar(LineNode* line, char ch);
    void buildClipList(const string& text);
    void cloneChain(CharNode* src, CharNode*& h, CharNode*& t);
    void insertChainAtCursor(CharNode* h, CharNode* t);
    void pasteClipList();
    void clearSkipList();
    void rebuildSkipList();
    LineNode* getLine(int lineNumber);
    int getCurrentColumn();
    int getLineLength(LineNode* line);
    void setCursorColumn(int column);
    void updateCurrentLineNumber();
public:
    TextEditor();
    ~TextEditor();
    string normalizeFilename(string filename);
    string readFilename();
    bool saveDocument(bool saveAs = false);
    bool openDocument();
    string getText();
    void setText(const string& content);
    void moveTo(int line, int column);
    bool getSelectionRange(int& sl, int& sc, int& el, int& ec);
    bool selectionActive();
    void beginSelection();
    void clearSelection();
    void selectAll();
    string textBetween(int sl, int sc, int el, int ec);
    bool deleteSelection();
    void setSystemClipboard(const string& text);
    bool getSystemClipboard(string& text);
    void copy();
    void cut();
    void paste();
    void insert(char ch);
    void moveLeft();
    void moveRight();
    void backspace();
    void deleteChar();
    void mergeWithPreviousLine();
    void mergeWithNextLine();
    void newLine();
    void moveUp();
    void moveDown();
    void flushBuffer(HANDLE hOut, string& buf);
    void clearScreen(HANDLE hOut);
    void display();
    void start();
    void showList();
};
#endif

#ifndef COMMAND_H
#define COMMAND_H

#include <stack>
#include <string>
#include <vector>

struct EditorSnapshot
{
    std::string text;
    std::vector<unsigned short> colors;
    int line;
    int column;
};

class Command
{
private:
    std::stack<EditorSnapshot> undoStack;
    std::stack<EditorSnapshot> redoStack;

public:
    void record(const EditorSnapshot& before, const EditorSnapshot& after);
    bool undo(const EditorSnapshot& current, EditorSnapshot& target);
    bool redo(const EditorSnapshot& current, EditorSnapshot& target);
    void clear();
};

#endif

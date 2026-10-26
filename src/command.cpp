#include "Command.h"

void Command::record(const EditorSnapshot& before, const EditorSnapshot& after)
{
    if (before.text == after.text)
        return;

    undoStack.push(before);
    while (!redoStack.empty())
        redoStack.pop();
}

bool Command::undo(const EditorSnapshot& current, EditorSnapshot& target)
{
    if (undoStack.empty())
        return false;

    redoStack.push(current);
    target = undoStack.top();
    undoStack.pop();
    return true;
}

bool Command::redo(const EditorSnapshot& current, EditorSnapshot& target)
{
    if (redoStack.empty())
        return false;

    undoStack.push(current);
    target = redoStack.top();
    redoStack.pop();
    return true;
}

void Command::clear()
{
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();
}

// Text editor typing engine using nested doubly linked lists.
//
//   Outer DLL : LineNode  <-> LineNode  <-> ...      (each node = one line)
//   Inner DLL : CharNode  <-> CharNode  <-> ...      (each LineNode owns one)
//
// Cursor = (LineNode* line, CharNode* at)
//   'at' is the character the cursor sits BEFORE.
//   at == nullptr  means the cursor is at the END of the line.
//
// Build: g++ -std=c++17 text_editor.cpp -o editor
#include <iostream>
#include <string>
using namespace std;

// ---------------------------------------------------------------- inner list
struct CharNode {
    char ch;
    CharNode *prev = nullptr, *next = nullptr;
    explicit CharNode(char c) : ch(c) {}
};

class CharList {
public:
    CharNode *head = nullptr, *tail = nullptr;
    int size = 0;

    ~CharList() { clear(); }

    void clear() {
        while (head) { CharNode* t = head; head = head->next; delete t; }
        tail = nullptr; size = 0;
    }

    // Insert new char just before 'pos' (pos == nullptr -> append at end).
    CharNode* insertBefore(CharNode* pos, char c) {
        CharNode* n = new CharNode(c);
        if (!pos) {                         // append
            n->prev = tail;
            if (tail) tail->next = n; else head = n;
            tail = n;
        } else {
            n->next = pos;
            n->prev = pos->prev;
            if (pos->prev) pos->prev->next = n; else head = n;
            pos->prev = n;
        }
        size++;
        return n;
    }

    void erase(CharNode* n) {
        if (!n) return;
        if (n->prev) n->prev->next = n->next; else head = n->next;
        if (n->next) n->next->prev = n->prev; else tail = n->prev;
        delete n;
        size--;
    }

    // Detach everything from 'pos' to the end and return it as a new list.
    // Used when Enter is pressed in the middle of a line.
    CharList splitAt(CharNode* pos) {
        CharList rest;
        if (!pos) return rest;
        rest.head = pos;
        rest.tail = tail;
        if (pos->prev) { pos->prev->next = nullptr; tail = pos->prev; }
        else           { head = tail = nullptr; }
        pos->prev = nullptr;
        int cnt = 0;
        for (CharNode* p = rest.head; p; p = p->next) cnt++;
        rest.size = cnt;
        size -= cnt;
        return rest;                        // (moved; see move-ctor note below)
    }

    // Join another list to the end of this one (other becomes empty).
    void append(CharList& other) {
        if (!other.head) return;
        if (tail) { tail->next = other.head; other.head->prev = tail; }
        else      { head = other.head; }
        tail = other.tail;
        size += other.size;
        other.head = other.tail = nullptr;
        other.size = 0;
    }

    // Allow splitAt to return by value without double-free.
    CharList() = default;
    CharList(CharList&& o) noexcept : head(o.head), tail(o.tail), size(o.size) {
        o.head = o.tail = nullptr; o.size = 0;
    }
    CharList& operator=(CharList&& o) noexcept {
        if (this != &o) {
            clear();
            head = o.head; tail = o.tail; size = o.size;
            o.head = o.tail = nullptr; o.size = 0;
        }
        return *this;
    }
    CharList(const CharList&) = delete;
    CharList& operator=(const CharList&) = delete;
};

// ---------------------------------------------------------------- outer list
struct LineNode {
    CharList chars;                         // inner doubly linked list
    LineNode *prev = nullptr, *next = nullptr;
};

class Editor {
    LineNode *first, *last;                 // outer list ends
    LineNode* line;                         // cursor: current line
    CharNode* at;                           // cursor: char to the right of cursor

    int column() const {                    // cursor column in current line
        int col = 0;
        for (CharNode* p = line->chars.head; p && p != at; p = p->next) col++;
        return col;
    }
    void gotoColumn(int col) {
        at = line->chars.head;
        while (col-- > 0 && at) at = at->next;
    }

public:
    Editor() {
        first = last = line = new LineNode();
        at = nullptr;
    }
    ~Editor() {
        while (first) { LineNode* t = first; first = first->next; delete t; }
    }

    // ---- typing -----------------------------------------------------------
    void insertChar(char c) {
        line->chars.insertBefore(at, c);    // cursor stays before 'at'
    }
    void insertText(const string& s) { for (char c : s) insertChar(c); }

    void enter() {                          // split current line at cursor
        LineNode* nl = new LineNode();
        nl->chars = line->chars.splitAt(at);
        nl->prev = line;
        nl->next = line->next;
        if (line->next) line->next->prev = nl; else last = nl;
        line->next = nl;
        line = nl;
        at = line->chars.head;              // cursor at start of new line
    }

    void backspace() {
        CharNode* victim = at ? at->prev : line->chars.tail;
        if (victim) { line->chars.erase(victim); return; }
        // At column 0: merge this line into the previous one.
        if (!line->prev) return;
        LineNode* old = line;
        CharNode* firstMoved = old->chars.head;
        LineNode* p = old->prev;
        p->chars.append(old->chars);
        p->next = old->next;
        if (old->next) old->next->prev = p; else last = p;
        delete old;
        line = p;
        at = firstMoved;                    // cursor at the join point
    }

    void del() {                            // Delete key
        if (at) {
            CharNode* victim = at;
            at = at->next;
            line->chars.erase(victim);
            return;
        }
        // At end of line: pull next line up.
        LineNode* nx = line->next;
        if (!nx) return;
        at = nx->chars.head;                // join point
        line->chars.append(nx->chars);
        line->next = nx->next;
        if (nx->next) nx->next->prev = line; else last = line;
        delete nx;
    }

    // ---- cursor movement --------------------------------------------------
    void left() {
        if (at != line->chars.head) { at = at ? at->prev : line->chars.tail; }
        else if (line->prev) { line = line->prev; at = nullptr; }
    }
    void right() {
        if (at) at = at->next;
        else if (line->next) { line = line->next; at = line->chars.head; }
    }
    void up() {
        if (!line->prev) return;
        int c = column(); line = line->prev; gotoColumn(c);
    }
    void down() {
        if (!line->next) return;
        int c = column(); line = line->next; gotoColumn(c);
    }
    void home() { at = line->chars.head; }
    void end()  { at = nullptr; }

    // ---- display ----------------------------------------------------------
    void print() const {
        cout << "-------------------------\n";
        int n = 1;
        for (LineNode* l = first; l; l = l->next, n++) {
            cout << n << ": ";
            for (CharNode* c = l->chars.head; c; c = c->next) {
                if (l == line && c == at) cout << '|';
                cout << c->ch;
            }
            if (l == line && !at) cout << '|';
            cout << '\n';
        }
        cout << "-------------------------\n";
    }
};

// ---------------------------------------------------------------- driver
int main() {
    Editor ed;
    cout << "Commands:\n"
            "  i <text>  insert text at cursor    b  backspace\n"
            "  d  delete                          n  enter (new line)\n"
            "  l/r/u/j  cursor left/right/up/down h  home    e  end\n"
            "  q  quit\n";
    ed.print();

    string input;
    while (cout << "> ", getline(cin, input)) {
        if (input.empty()) continue;
        char cmd = input[0];
        if (cmd == 'q') break;
        switch (cmd) {
            case 'i': if (input.size() > 2) ed.insertText(input.substr(2)); break;
            case 'b': ed.backspace(); break;
            case 'd': ed.del();       break;
            case 'n': ed.enter();     break;
            case 'l': ed.left();      break;
            case 'r': ed.right();     break;
            case 'u': ed.up();        break;
            case 'j': ed.down();      break;
            case 'h': ed.home();      break;
            case 'e': ed.end();       break;
            default:  cout << "Unknown command\n"; continue;
        }
        ed.print();
    }
    return 0;
}

#include "FileHandler.h"

#include <fstream>
#include <string>
#include <windows.h>
#include <commdlg.h>
#include <string>
#pragma comment(lib, "Comdlg32.lib")
bool saveToFile(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename.c_str(),std::ios::binary);

    if (!file.is_open())
        return false;

    file.write(content.data(),static_cast<std::streamsize>(content.size()));

    if (!file.good())
        return false;

    file.close();
    return !file.fail();
}

bool loadFromFile(const std::string& filename,std::string& content)
{
    std::ifstream file(filename.c_str(), std::ios::binary);

    if (!file.is_open())
        return false;

    std::string temp(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    if (file.bad())
        return false;

    // Convert Windows CRLF line endings to LF.
    std::string normalized;
    normalized.reserve(temp.size());

    for (std::size_t i = 0; i < temp.size(); ++i)
    {
        if (temp[i] == '\r' &&
            i + 1 < temp.size() &&
            temp[i + 1] == '\n')
        {
            continue;
        }

        normalized += temp[i];
    }

    content = normalized;
    return true;
}

bool chooseOpenFilename(std::string& filename)
{
    char filePath[MAX_PATH] = "";

    OPENFILENAMEA ofn = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetConsoleWindow();
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;

    ofn.lpstrFilter =
        "Text Files (*.txt)\0*.txt\0"
        "All Files (*.*)\0*.*\0";

    ofn.nFilterIndex = 1;

    ofn.Flags =
        OFN_FILEMUSTEXIST |
        OFN_PATHMUSTEXIST |
        OFN_HIDEREADONLY;

    if (GetOpenFileNameA(&ofn))
    {
        filename = filePath;
        return true;
    }

    return false;
}
bool chooseSaveFilename(std::string& filename)
{
    char filePath[MAX_PATH] = "Untitled.txt";

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetConsoleWindow();
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter =
        "Text Files (*.txt)\0*.txt\0"
        "All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = "txt";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameA(&ofn))
    {
        filename = filePath;
        return true;
    }

    return false;
}
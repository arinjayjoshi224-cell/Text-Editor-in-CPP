#include "FileHandler.h"

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <windows.h>
#include <commdlg.h>

#pragma comment(lib, "Comdlg32.lib")

static const std::string META_HEADER = "TEXT_EDITOR_METADATA_V1";
static const std::string META_END = "END_TEXT_EDITOR_METADATA";

static std::string normalizeLineEndings(const std::string& text)
{
    std::string result;
    result.reserve(text.size());

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == '\r' &&
            i + 1 < text.size() &&
            text[i + 1] == '\n')
        {
            continue;
        }

        result += text[i];
    }

    return result;
}

bool saveToFile(
    const std::string& filename,
    const std::string& content)
{
    std::ofstream file(filename.c_str(), std::ios::binary);

    if (!file.is_open())
        return false;

    file.write(
        content.data(),
        static_cast<std::streamsize>(content.size())
    );

    if (!file.good())
        return false;

    file.close();
    return !file.fail();
}

bool saveToFile(const std::string& filename, const std::string& content, const std::vector<unsigned short>& colors)
{
    std::ofstream file(filename.c_str(), std::ios::binary);

    if (!file.is_open())
        return false;

    file << META_HEADER << '\n';

    for (std::size_t i = 0; i < colors.size(); ++i)
    {
        if (i > 0)
            file << ',';

        file << colors[i];
    }

    file << '\n' << META_END << '\n';
    file.write(
        content.data(),
        static_cast<std::streamsize>(content.size())
    );

    if (!file.good())
        return false;

    file.close();
    return !file.fail();
}

bool loadFromFile(const std::string& filename, std::string& content)
{
    std::vector<unsigned short> ignoredColors;
    return loadFromFile(filename, content, ignoredColors);
}

bool loadFromFile(
    const std::string& filename,
    std::string& content,
    std::vector<unsigned short>& colors)
{
    std::ifstream file(filename.c_str(), std::ios::binary);

    if (!file.is_open())
        return false;

    std::string raw(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    if (file.bad())
        return false;

    std::string normalized = normalizeLineEndings(raw);

    // Files without our metadata remain ordinary text files.
    const std::size_t firstNewline = normalized.find('\n');

    if (firstNewline == std::string::npos ||
        normalized.substr(0, firstNewline) != META_HEADER)
    {
        content = normalized;
        colors.clear();
        return true;
    }

    const std::size_t secondNewline =
        normalized.find('\n', firstNewline + 1);

    if (secondNewline == std::string::npos)
        return false;

    const std::size_t thirdNewline =
        normalized.find('\n', secondNewline + 1);

    if (thirdNewline == std::string::npos ||
        normalized.substr(
            secondNewline + 1,
            thirdNewline - secondNewline - 1
        ) != META_END)
    {
        return false;
    }

    std::string colorData = normalized.substr(
        firstNewline + 1,
        secondNewline - firstNewline - 1
    );

    std::vector<unsigned short> loadedColors;
    std::stringstream stream(colorData);
    std::string token;

    while (std::getline(stream, token, ','))
    {
        if (token.empty())
            return false;

        char* end = NULL;
        unsigned long value = std::strtoul(token.c_str(), &end, 10);

        if (end == token.c_str() ||
            *end != '\0' ||
            value > 15)
        {
            return false;
        }

        loadedColors.push_back(
            static_cast<unsigned short>(value)
        );
    }

    content = normalized.substr(thirdNewline + 1);
    colors = loadedColors;

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
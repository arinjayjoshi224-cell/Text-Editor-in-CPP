
#include "FileHandler.h"

#include <fstream>
#include <string>

bool saveToFile(const std::string& filename,
                const std::string& content)
{
    std::ofstream file(filename.c_str(),
                       std::ios::binary);

    if (!file.is_open())
        return false;

    file.write(content.data(),
               static_cast<std::streamsize>(content.size()));

    if (!file.good())
        return false;

    file.close();
    return !file.fail();
}

bool loadFromFile(const std::string& filename,
                  std::string& content)
{
    std::ifstream file(filename.c_str(),
                       std::ios::binary);

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

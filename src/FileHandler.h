
#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>

bool saveToFile(const std::string& filename, const std::string& content, const std::vector<unsigned short>& colors);

bool loadFromFile(const std::string& filename, std::string& content, std::vector<unsigned short>& colors);

bool chooseOpenFilename(std::string& filename);

bool chooseSaveFilename(std::string& filename);

#endif

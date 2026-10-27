
#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>

bool saveToFile(const std::string& filename,const std::string& content);

bool loadFromFile(const std::string& filename,std::string& content);

bool chooseOpenFilename(std::string& filename);

bool chooseSaveFilename(std::string& filename);

#endif

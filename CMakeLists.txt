cmake_minimum_required(VERSION 3.16)
project(TextEditor LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_add_executable(TextEditor
    src/main.cpp
    src/mainWindow.cpp
    src/mainWindow.h
)
target_link_libraries(TextEditor PRIVATE Qt5::Widgets)
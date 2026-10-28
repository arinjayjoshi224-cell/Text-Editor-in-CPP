#ifndef CONSOLEHELPERS_H
#define CONSOLEHELPERS_H
#pragma once
#include <windows.h>

void setTextColor(WORD color);
void resetTextColor();
void gotoxy(int x,int y);
BOOL WINAPI ctrlHandler(DWORD type);
#endif

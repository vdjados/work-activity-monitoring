#pragma once
#include <vector>
#include <windows.h>

class Screenshot {
public:
    static std::vector<BYTE> Capture();
};
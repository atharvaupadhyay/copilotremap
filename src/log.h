#pragma once
#include <windows.h>
#include <stdio.h>

#ifdef _DEBUG
inline void LogDebug(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    OutputDebugStringA(buffer);
}
#define LOG(...) LogDebug(__VA_ARGS__)
#else
#define LOG(...)
#endif

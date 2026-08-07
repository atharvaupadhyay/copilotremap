#pragma once
#include <windows.h>
#include <cstdarg>
#include <cstdio>

#ifdef _DEBUG

inline void LogDebug(const char* format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);

#if defined(_MSC_VER)
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
#else
    vsnprintf(buffer, sizeof(buffer), format, args);
    buffer[sizeof(buffer) - 1] = '\0';
#endif

    va_end(args);

    // DebugView / Visual Studio Output window
    OutputDebugStringA(buffer);

    // Also print to stdout (useful for console builds)
    std::fputs(buffer, stdout);
    std::fflush(stdout);
}

#define LOG(...) LogDebug(__VA_ARGS__)

#else

#define LOG(...) ((void)0)

#endif
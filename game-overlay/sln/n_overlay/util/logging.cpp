#include "logging.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

thread_local bool g_bInLoggingFlow;
std::string       g_pLoggerMapName;

void FatalError(const char* message, ...)
{
    char    text[2048];
    va_list args;
    va_start(args, message);
    vsnprintf(text, sizeof(text), message, args);
    va_end(args);

    OutputDebugStringA("[n_overlay] Fatal: ");
    OutputDebugStringA(text);
    OutputDebugStringA("\n");

#ifdef PLATFORM_WINDOWS
    if (IsDebuggerPresent())
        DebugBreak();
#endif
}

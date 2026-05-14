#pragma once

#include "platform.h"

#include <string>

extern std::string       g_pLoggerMapName;
extern thread_local bool g_bInLoggingFlow;

class Color
{
public:
    Color() { *reinterpret_cast<int*>(this) = 0; }
    Color(int _r, int _g, int _b) { SetColor(_r, _g, _b, 0); }
    Color(int _r, int _g, int _b, int _a) { SetColor(_r, _g, _b, _a); }

    void SetColor(int _r, int _g, int _b, int _a = 0)
    {
        _color[0] = static_cast<unsigned char>(_r);
        _color[1] = static_cast<unsigned char>(_g);
        _color[2] = static_cast<unsigned char>(_b);
        _color[3] = static_cast<unsigned char>(_a);
    }
    unsigned char r() const { return _color[0]; }
    unsigned char g() const { return _color[1]; }
    unsigned char b() const { return _color[2]; }
    unsigned char a() const { return _color[3]; }

private:
    unsigned char _color[4];
};

// Simplified for injected DLL — use OutputDebugStringA, no game engine ConColorMsg
void FatalError(const char* message, ...);

#define AssertPtr(ptr)                 \
    if ((ptr) == nullptr) [[unlikely]] \
    FatalError("MS: %s is nullptr in %s\n", #ptr, __FUNCTION__)

#define AssertBool(v)      \
    if (!(v)) [[unlikely]] \
    FatalError("MS: %s is false in %s\n", #v, __FUNCTION__)

#define AssertVal(v)       \
    if (!(v)) [[unlikely]] \
    FatalError("MS: %s is default in %s\n", #v, __FUNCTION__)

#define BooleanSTR(v) (v ? "true" : "false")

#define LOG(...)
#define WARN(...)
#define FLOG(...)
#define FERROR(...)

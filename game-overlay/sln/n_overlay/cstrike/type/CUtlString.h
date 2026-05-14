// This file modified from https://github.com/alliedmodders/hl2sdk/tree/cs2

#ifndef CSTRIKE_TYPE_CUTLSTRING_H
#define CSTRIKE_TYPE_CUTLSTRING_H

#include "platform.h"

class CUtlString
{
public:
    using TUtlStringPattern = enum {
        PATTERN_NONE      = 0x00000000,
        PATTERN_DIRECTORY = 0x00000001
    };

    using AltArgumentType_t = const char*;

    CUtlString();
    CUtlString(const char* pString);
    CUtlString(const char* pString, int length);
    CUtlString(const CUtlString& string);

    ~CUtlString();

    MS_CLASS_IMPORT bool operator==(const CUtlString& src) const;
    bool                 operator!=(const CUtlString& src) const { return !operator==(src); }

    CUtlString& operator=(const CUtlString& src);
    CUtlString& operator=(const char* src);

    MS_CLASS_IMPORT char operator[](int i) const;

    MS_CLASS_IMPORT CUtlString operator+(const char* pOther) const;
    MS_CLASS_IMPORT CUtlString operator+(const CUtlString& other) const;
    MS_CLASS_IMPORT CUtlString operator+(int rhs) const;

    MS_CLASS_IMPORT CUtlString& operator+=(const CUtlString& rhs);
    MS_CLASS_IMPORT CUtlString& operator+=(char c);
    MS_CLASS_IMPORT CUtlString& operator+=(int rhs);
    MS_CLASS_IMPORT CUtlString& operator+=(double rhs);
    MS_CLASS_IMPORT CUtlString& operator+=(const char* rhs);

    const char* Get() const;

    operator const char*() const { return Get(); }
    const char* String() const { return Get(); }

    MS_CLASS_IMPORT void Set(const char* pValue);
    MS_CLASS_IMPORT void SetDirect(const char* pValue, int length);
    MS_CLASS_IMPORT void Append(const char* pchAddition);

    int Length() const;

    bool IsEmpty() const;

    MS_CLASS_IMPORT void Purge();

    MS_CLASS_IMPORT void ToLowerFast();
    MS_CLASS_IMPORT void ToUpperFast();

private:
    char* m_pString;
};

MS_EXPORT_C_API void UtlString_Set(CUtlString* pThis, const char* string);
MS_EXPORT_C_API void UtlString_SetDirect(CUtlString* pThis, const char* string, int length);
MS_EXPORT_C_API void UtlString_Purge(CUtlString* pThis);

#endif
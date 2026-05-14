/*
 * Minimal CUtlString implementation for n_overlay.
 * The game engine provides full implementations via dllimport;
 * we stub the subset needed by CModule's vtable scanning.
 */

#include "CUtlString.h"
#include <cstring>
#include <cstdlib>

CUtlString::CUtlString() :
    m_pString(nullptr) {}

CUtlString::CUtlString(const char* pString) :
    m_pString(pString ? _strdup(pString) : nullptr) {}

CUtlString::CUtlString(const char* pString, int length) :
    m_pString(nullptr)
{
    if (pString) {
        m_pString = (char*)malloc(length + 1);
        if (m_pString) {
            memcpy(m_pString, pString, length);
            m_pString[length] = 0;
        }
    }
}

CUtlString::CUtlString(const CUtlString& string) :
    m_pString(string.m_pString ? _strdup(string.m_pString) : nullptr) {}

CUtlString::~CUtlString()
{
    free(m_pString);
    m_pString = nullptr;
}

CUtlString& CUtlString::operator=(const CUtlString& src)
{
    if (this != &src) {
        free(m_pString);
        m_pString = src.m_pString ? _strdup(src.m_pString) : nullptr;
    }
    return *this;
}

CUtlString& CUtlString::operator=(const char* src)
{
    free(m_pString);
    m_pString = src ? _strdup(src) : nullptr;
    return *this;
}

const char* CUtlString::Get() const
{
    return m_pString ? m_pString : "";
}

int CUtlString::Length() const
{
    return m_pString ? (int)strlen(m_pString) : 0;
}

bool CUtlString::IsEmpty() const
{
    return !m_pString || !m_pString[0];
}

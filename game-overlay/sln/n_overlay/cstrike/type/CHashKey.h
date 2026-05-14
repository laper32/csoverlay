/* 
 * ModSharp
 * Copyright (C) 2023-2026 Kxnrl. All Rights Reserved.
 *
 * This file is part of ModSharp.
 * ModSharp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ModSharp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with ModSharp. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CSTRIKE_TYPE_HASHKEY_H
#define CSTRIKE_TYPE_HASHKEY_H

#include "murmurhash.h"

#include <cstdint>

struct CHashKey
{
public:
    CHashKey(const char* key) :
        m_nHashCode(0), m_pszKeyName("")
    {
        if (!key || !key[0])
            return;

        m_pszKeyName = key;
        m_nHashCode  = MurmurHash2Lowercase(key, MURMURHASH_SEED);
    }

    CHashKey() = delete;

    [[nodiscard]] uint32_t    GetHashCode() const { return m_nHashCode; }
    [[nodiscard]] const char* GetKeyName() const { return m_pszKeyName; }

private:
    uint32_t    m_nHashCode;
    const char* m_pszKeyName;
};

struct CUtlStringToken
{
public:
    CUtlStringToken(uint32_t nHashCode = 0) :
        m_nHashCode(nHashCode) {}
    CUtlStringToken(const char* str) :
        m_nHashCode(0)
    {
        if (str && *str)
        {
            m_nHashCode = MurmurHash2Lowercase(str, MURMURHASH_SEED);
        }
    }

    bool operator==(const CUtlStringToken& other) const { return (other.m_nHashCode == m_nHashCode); }
    bool operator!=(const CUtlStringToken& other) const { return !operator==(other); }
    bool operator<(const CUtlStringToken& other) const { return (m_nHashCode < other.m_nHashCode); }

    [[nodiscard]] bool     IsValid() const { return m_nHashCode != 0; }
    [[nodiscard]] uint32_t GetHashCode() const { return m_nHashCode; }

private:
    uint32_t m_nHashCode;
};
#endif
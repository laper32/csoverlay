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

#ifndef CSTRIKE_TYPE_CUTLSOURCELOCATION_H
#define CSTRIKE_TYPE_CUTLSOURCELOCATION_H

#include <cstdint>

class CUtlSourceLocation
{   
    const char* m_pFileName;
    uint64_t    m_nLineNumber;
    const char* m_pFunctionName;

public:
    constexpr CUtlSourceLocation(const char* file, const char* func, int line) :
        m_pFileName(file),
        m_nLineNumber(line),
        m_pFunctionName(func)
    {
    }
};

#endif
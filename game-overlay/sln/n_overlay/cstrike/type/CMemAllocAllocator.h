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


#ifndef CSTRIKE_TYPE_MEM_ALLOC_ALLOCATOR_H
#define CSTRIKE_TYPE_MEM_ALLOC_ALLOCATOR_H

#include "global.h"

#include <algorithm>
#include <limits>

class CMemAllocAllocator
{
public:
    static void* Alloc(size_t nSize)
    {
        return AllocateMemory(nSize);
    }

    static void* Realloc(void* base, size_t nSize)
    {
        return ReallocateMemory(base, nSize);
    }

    static void Free(void* pMem)
    {
        if (pMem)
            FreeMemory(pMem);
    }

    static size_t GetSize(void* pMem)
    {
        return GetMemorySize(pMem);
    }
};
#endif
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

#ifndef MS_ROOT_GLOBAL_H
#define MS_ROOT_GLOBAL_H

#include "definitions.h"
#include "platform.h"
#include "types.h"

class IMemAlloc;
class IGameEventManager2;

using CreateInterface_t = void* (*)(const char* name, int* ret);

// Memory allocator — resolved at runtime from tier0.dll
extern IMemAlloc* g_pMemAlloc;

void*  AllocateMemory(size_t size);
void   FreeMemory(void* mem);
void*  ReallocateMemory(void* mem, size_t size);
size_t GetMemorySize(void* mem);

// Game event manager — resolved by eventhook.cpp
extern IGameEventManager2* g_pEventManager;

#endif

#include "memory_access.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

bool SetMemoryAccess(uint8_t* address, size_t size, uint8_t access, uint8_t* old_access)
{
    DWORD new_protect = 0;
    if (access & MemoryAccess_Execute) {
        if (access & MemoryAccess_Write) new_protect = PAGE_EXECUTE_READWRITE;
        else if (access & MemoryAccess_Read) new_protect = PAGE_EXECUTE_READ;
        else new_protect = PAGE_EXECUTE;
    } else {
        if (access & MemoryAccess_Write) new_protect = PAGE_READWRITE;
        else if (access & MemoryAccess_Read) new_protect = PAGE_READONLY;
        else new_protect = PAGE_NOACCESS;
    }

    DWORD old;
    if (!VirtualProtect(address, size, new_protect, &old))
        return false;

    if (old_access != nullptr) {
        if (old == PAGE_EXECUTE_READWRITE)
            *old_access = MemoryAccess_Read | MemoryAccess_Write | MemoryAccess_Execute;
        else if (old == PAGE_EXECUTE_READ)
            *old_access = MemoryAccess_Read | MemoryAccess_Execute;
        else if (old == PAGE_READWRITE)
            *old_access = MemoryAccess_Read | MemoryAccess_Write;
        else if (old == PAGE_READONLY)
            *old_access = MemoryAccess_Read;
        else
            *old_access = 0;
    }
    return true;
}

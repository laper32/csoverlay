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

#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS
#endif
#ifdef PLATFORM_WINDOWS

#    include "platform.h"
#    include "logging.h"
#    include "module.h"
#    include "scopetimer.h"

#    include <algorithm>
#    include <cinttypes>
#    include <ranges>

#    include <windows.h>
#    include <winternl.h>

#    ifdef __clang__
#        define _ThrowInfo ThrowInfo
#    endif
#    include <rttidata.h>
#    ifdef __clang__
#        undef _ThrowInfo
#    endif
#    include <vcruntime.h>

void CModule::GetModuleInfo(std::string_view mod)
{
    HMODULE handle = GetModuleHandleA(mod.data());
    if (!handle)
        return;

    _module_name.resize(MAX_PATH);
    auto actual_size = GetModuleFileNameA(handle, _module_name.data(), MAX_PATH);
    _module_name.resize(actual_size);

    _module_name = _module_name.substr(_module_name.find_last_of('\\') + 1);

    _base_address = reinterpret_cast<uintptr_t>(handle);

    const auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);

    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        return;

    const auto bytes = reinterpret_cast<uint8_t*>(handle);

    const auto ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(bytes + dosHeader->e_lfanew);

    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
        return;

    _size = ntHeader->OptionalHeader.SizeOfImage;

    auto section = IMAGE_FIRST_SECTION(ntHeader);

    for (auto i = 0u; i < ntHeader->FileHeader.NumberOfSections; i++, section++)
    {
        const auto isExecutable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        const auto isReadable   = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;
        const auto isWritable   = (section->Characteristics & IMAGE_SCN_MEM_WRITE) != 0;

        const auto start = this->_base_address + section->VirtualAddress;
        const auto size  = section->Misc.VirtualSize;

        auto& segment   = _segments.emplace_back();
        segment.address = start;
        segment.size    = size;
        if (isExecutable)
            segment.flags |= FLAG_X;
        if (isReadable)
            segment.flags |= FLAG_R;
        if (isWritable)
            segment.flags |= FLAG_W;

        const auto data = reinterpret_cast<uint8_t*>(start);
        segment.data    = std::vector(data, data + size);
    }

    _createInterFaceFn = GetFunctionByName("CreateInterface");

    {
        ScopedTimer timer(_module_name + "::DumpVTables");
        DumpVtables();
    }
    // BuildFunctionIndexAndReferences not needed for n_overlay — we only use vtable scanning
}

// BuildFunctionIndexAndReferences removed — not needed for n_overlay vtable scanning

void CModule::DumpVtables()
{
    // originally inspired by praydog & cursey's kananlib https://github.com/cursey/kananlib/blob/main/src/RTTI.cpp
    constexpr auto type_info_type_descriptor_name = ".?AVtype_info@@";

    auto type_descriptor_address = FindString(type_info_type_descriptor_name, false);
    if (!type_descriptor_address.IsValid())
    {
        FERROR("Failed to find type descriptor address for \"%s\" in module %s", type_info_type_descriptor_name, _module_name.c_str());
        return;
    }

    auto type_info = type_descriptor_address.Offset(-0x10).Dereference();

    const auto type_info_xrefs = FindPtrs(type_info);
    _vtables.reserve(type_info_xrefs.size());

    std::vector<uint32_t> valid_type_rvas;
    valid_type_rvas.reserve(type_info_xrefs.size());

    for (auto xref : type_info_xrefs) valid_type_rvas.push_back(static_cast<uint32_t>(xref.GetPtr() - _base_address));

    std::sort(valid_type_rvas.begin(), valid_type_rvas.end());

    for (const auto& segment : _segments)
    {
        if (segment.flags & (FLAG_X | FLAG_W)) continue;

        auto start_addr = segment.address;
        auto end_addr   = start_addr + segment.size;

        auto is_in_current_segment = [&](uintptr_t ptr) {
            return start_addr <= ptr && ptr < end_addr;
        };

        for (uintptr_t ptr = start_addr; ptr < end_addr - sizeof(void*); ptr += sizeof(void*))
        {
            uintptr_t potential_col_ptr = *reinterpret_cast<uintptr_t*>(ptr);

            if ((potential_col_ptr & 3) != 0) continue;
            if (!is_in_current_segment(potential_col_ptr)) continue;

            auto col = reinterpret_cast<_s_RTTICompleteObjectLocator*>(potential_col_ptr);

            if (col->signature != 1) continue;

            if (std::binary_search(valid_type_rvas.begin(), valid_type_rvas.end(), (uint32_t)(uintptr_t)(col->pTypeDescriptor)))
            {
                uintptr_t vtable_start = ptr + sizeof(void*);
                auto      ti           = reinterpret_cast<std::type_info*>(_base_address + col->pTypeDescriptor);

                auto node = std::make_unique<VTable>(ti, vtable_start, ti->name(), col->offset, col);

                _vtables.push_back(std::move(node));
            }
        }
    }

    // Hierarchy walking removed — not needed for our use case (finding CGameEventManager singleton).
    // MSVC RTTI types differ between SDK versions, causing build errors.
}

CAddress CModule::GetFunctionByName(std::string_view proc_name) const
{
    return GetProcAddress(reinterpret_cast<HMODULE>(_base_address), proc_name.data());
}

#endif

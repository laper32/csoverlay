/*
 * Based on ModSharp event hooking infrastructure.
 * Copyright (C) 2023-2026 Kxnrl. All Rights Reserved.
 *
 * Adapted for goverlay n_overlay — client-side CS2 game event listener.
 */

#include "eventhook.h"

#include "global.h"
#include "module.h"

#include "cstrike/interface/IGameEvent.h"
#include "cstrike/interface/IMemAlloc.h"

#include <windows.h>

// Minimal x64 inline hook — no external dependencies
static uint8_t  g_OrigBytes[14];
static uint8_t* g_HookTarget    = nullptr;
static uint8_t* g_Trampoline    = nullptr;  // callable original

static void InstallHook(void* target, void* detour)
{
    g_HookTarget = (uint8_t*)target;
    memcpy(g_OrigBytes, target, 14);

    // Allocate trampoline: orig bytes + jmp back to target+14
    g_Trampoline = (uint8_t*)VirtualAlloc(nullptr, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memcpy(g_Trampoline, g_OrigBytes, 14);
    // jmp [rip] — jmp back to original + 14
    uint8_t* jmpBack = g_HookTarget + 14;
    g_Trampoline[14] = 0xFF; g_Trampoline[15] = 0x25; g_Trampoline[16] = 0x00; g_Trampoline[17] = 0x00; g_Trampoline[18] = 0x00; g_Trampoline[19] = 0x00;
    memcpy(g_Trampoline + 20, &jmpBack, 8);

    // Patch target: mov rax, imm64; jmp rax -> detour
    DWORD old;
    VirtualProtect(target, 14, PAGE_EXECUTE_READWRITE, &old);
    uint8_t tramp[] = { 0x48, 0xB8, 0,0,0,0,0,0,0,0, 0xFF, 0xE0 };
    memcpy(tramp + 2, &detour, 8);
    memcpy(target, tramp, 12);
    VirtualProtect(target, 14, old, &old);
}

static void RemoveHook()
{
    if (!g_HookTarget) return;
    DWORD old;
    VirtualProtect(g_HookTarget, 14, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_HookTarget, g_OrigBytes, 14);
    VirtualProtect(g_HookTarget, 14, old, &old);
    if (g_Trampoline) { VirtualFree(g_Trampoline, 0, MEM_RELEASE); g_Trampoline = nullptr; }
    g_HookTarget = nullptr;
}
#include <tlhelp32.h>
#include <string>
#include <cstdio>
#include <cstring>
#include <io.h>
#include <fcntl.h>

static FILE* g_DbgConsole = nullptr;

static void dbgPrint(const char* fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    OutputDebugStringA(buf);
    if (g_DbgConsole) {
        fwrite(buf, 1, len, g_DbgConsole);
        fflush(g_DbgConsole);
    }
}

static void allocConsole()
{
    if (AllocConsole()) {
        freopen_s(&g_DbgConsole, "CONOUT$", "w", stdout);
        SetConsoleTitleA("n_overlay debug");
        dbgPrint("[n_overlay] Console allocated\n");
    }
}

GameEventCallback g_onGameEvent   = nullptr;
static const char* g_StatusMsg = "not initialized";

const char* getEventHookStatus() { return g_StatusMsg; }

// ---- Signature-scan + detour the client-side event dispatcher ----

using EventDispatchFn = void (*)(void* a1, IGameEvent* a2);
static EventDispatchFn g_OriginalDispatch = nullptr;

static void HookEventDispatch(void* a1, IGameEvent* event)
{
    dbgPrint("[n_overlay] EventDispatch called: a1=0x%p event=0x%p\n", a1, (void*)event);

    if (event) {
        const char* name = event->GetName();
        dbgPrint("[n_overlay]   event name: %s\n", name ? name : "(null)");
    }

    // Call original via trampoline
    ((EventDispatchFn)g_Trampoline)(a1, event);
}

static void dumpLoadedModules()
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hSnap == INVALID_HANDLE_VALUE) return;
    MODULEENTRY32 me; me.dwSize = sizeof(me);
    dbgPrint("[n_overlay] ---- Loaded modules ----\n");
    if (Module32First(hSnap, &me)) {
        do { dbgPrint("  %ls  (0x%p)\n", me.szModule, me.modBaseAddr); }
        while (Module32Next(hSnap, &me));
    }
    dbgPrint("[n_overlay] ------------------------\n");
    CloseHandle(hSnap);
}

bool initEventHook()
{
    allocConsole();
    dumpLoadedModules();

    // 1. Resolve g_pMemAlloc from tier0.dll
    {
        HMODULE tier0 = GetModuleHandleA("tier0.dll");
        if (!tier0) { g_StatusMsg = "tier0.dll not loaded"; return false; }
        auto* ppMemAlloc = (IMemAlloc**)GetProcAddress(tier0, "g_pMemAlloc");
        if (!ppMemAlloc) { g_StatusMsg = "g_pMemAlloc not found"; return false; }
        g_pMemAlloc = *ppMemAlloc;
        dbgPrint("[n_overlay] g_pMemAlloc=0x%p\n", (void*)g_pMemAlloc);
    }

    // 2. Load client.dll & scan for event dispatch function
    {
        LoadLibraryW(L"client.dll");
        HMODULE hClient = GetModuleHandleA("client.dll");
        if (!hClient) { g_StatusMsg = "client.dll not loaded"; return false; }

        auto* mod = new CModule("client.dll");
        dbgPrint("[n_overlay] client.dll base=0x%p\n", (void*)mod->Base());

        // Signature: 40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 41
        auto addr = mod->FindPattern(
            "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 41");
        if (!addr.IsValid()) {
            g_StatusMsg = "event dispatch pattern not found in client.dll";
            dbgPrint("[n_overlay] Pattern not found\n");
            delete mod;
            return false;
        }

        g_OriginalDispatch = (EventDispatchFn)addr.GetPtr();
        dbgPrint("[n_overlay] EventDispatch=0x%p\n", (void*)g_OriginalDispatch);
    }

    // 3. Install inline hook
    InstallHook((void*)g_OriginalDispatch, (void*)HookEventDispatch);
    dbgPrint("[n_overlay] Hook installed\n");

    g_StatusMsg = "ok";
    dbgPrint("[n_overlay] SUCCESS\n");
    return true;
}

void shutdownEventHook()
{
    RemoveHook();
    g_OriginalDispatch = nullptr;
    g_onGameEvent = nullptr;
}

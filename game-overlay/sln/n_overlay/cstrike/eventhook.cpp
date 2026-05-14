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

IGameEventManager2* g_pEventManager = nullptr;
GameEventCallback   g_onGameEvent   = nullptr;

static const char* g_StatusMsg = "not initialized";

const char* getEventHookStatus()
{
    return g_StatusMsg;
}

class CGameEventListener : public IGameEventListener2
{
public:
    void FireGameEvent(IGameEvent* event) override
    {
        if (!event) return;
        const char* name = event->GetName();
        if (!name) return;

        dbgPrint("[n_overlay] FireGameEvent: %s\n", name);

        if (strcmp(name, "round_mvp") != 0) return;

        int userid = 0, reason = 0;
        GameEventKeySymbol_t keyUserid("userid");
        GameEventKeySymbol_t keyReason("reason");
        if (event->HasKey(keyUserid)) userid = event->GetInt(keyUserid);
        if (event->HasKey(keyReason)) reason = event->GetInt(keyReason);

        char buf[128];
        snprintf(buf, sizeof(buf), "{\"userid\":%d,\"reason\":%d}", userid, reason);
        dbgPrint("[n_overlay] round_mvp! %s\n", buf);

        if (g_onGameEvent) g_onGameEvent("round_mvp", buf);
    }

    int GetEventDebugID(void) override { return 42; }
} g_EventListener;


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
    }

    dbgPrint("[n_overlay] g_pMemAlloc=0x%p\n", (void*)g_pMemAlloc);
    dbgPrint("[n_overlay] Loading client.dll");
    auto client = LoadLibraryW(L"client.dll");
    if (!client) {
        g_StatusMsg = "client.dll failed to load";
        dbgPrint("[n_overlay] client.dll failed to load\n");
        return false;
    }

    auto* mod = new CModule("client.dll");
    if (!mod) {
        g_StatusMsg = "CModule init failed";
        dbgPrint("[n_overlay] CModule init failed\n");
        return false;
    }

    dbgPrint("[n_overlay] client.dll base=0x%p\n", (void*)mod->Base());

    auto gameEventManagerVTable = mod->GetVirtualTableByName("CGameEventManager");
    if (!gameEventManagerVTable) {
        g_StatusMsg = "CGameEventManager vtable not found";
        dbgPrint("[n_overlay] CGameEventManager vtable not found\n");
        delete mod;
        return false;
    }

    g_pEventManager = mod->FindPtr(gameEventManagerVTable).As<IGameEventManager2*>();
    if (!g_pEventManager) {
        g_StatusMsg = "IGameEventManager2 instance not found";
        dbgPrint("[n_overlay] IGameEventManager2 instance not found\n");
        delete mod;
        return false;
    }

    dbgPrint("[n_overlay] CGameEventManager=0x%p\n", (void*)g_pEventManager);

    auto found = g_pEventManager->FindListener(&g_EventListener, "round_mvp");
    if (found) {
        g_StatusMsg = "Listener already registered?";
        dbgPrint("[n_overlay] Listener already registered?\n");
        delete mod;
        return false;
    }

    int eventId = g_pEventManager->AddListener(&g_EventListener, "round_start", false);
    if (eventId == -1) {
        g_StatusMsg = "AddListener failed: invalid event name?";
        dbgPrint("[n_overlay] AddListener failed: round_start is invalid?\n");
        delete mod;
        return false;
    }

    return true;
}

void shutdownEventHook()
{
    if (g_pEventManager) {
        g_pEventManager->RemoveListener(&g_EventListener);
        g_pEventManager = nullptr;
    }
    g_onGameEvent = nullptr;
}

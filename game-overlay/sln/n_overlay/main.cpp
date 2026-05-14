#include "stable.h"
#include "./overlay/session.h"
#include "./overlay/hookapp.h"
#include "./cstrike/eventhook.h"

HHOOK g_injectHook = nullptr;

static void onGameEventCallback(const char* eventName, const char* eventData)
{
    auto connector = HookApp::instance()->overlayConnector();
    if (connector && connector->isConnected())
    {
        connector->sendGameEvent(eventName, eventData);
    }
}

static DWORD WINAPI initEventHookThread(LPVOID)
{
    g_onGameEvent = onGameEventCallback;
    bool ok = initEventHook();

    // Wait for IPC to be ready, then send init status
    for (int i = 0; i < 30; i++) {
        auto connector = HookApp::instance()->overlayConnector();
        if (connector && connector->isConnected()) {
            connector->sendGameEvent("init_status", ok ? "ok" : getEventHookStatus());
            break;
        }
        Sleep(500);
    }
    return 0;
}

extern "C" __declspec(dllexport) LRESULT CALLBACK msg_hook_proc_ov(int code,
                                                                   WPARAM wparam, LPARAM lparam)
{
    static bool hooking = true;
    MSG *msg = (MSG *)lparam;

    if (hooking && msg->message == (WM_USER + 432))
    {
        typedef BOOL(WINAPI * fn)(HHOOK);

        g_injectHook = (HHOOK)msg->lParam;

        LOGGER("n_overlay") << "@trace threadId:" << ::GetCurrentThreadId() << ", window: " << msg->hwnd;

        session::setInjectWindow(msg->hwnd);
    }

    return CallNextHookEx(0, code, wparam, lparam);
}

HINSTANCE g_moduleHandle = nullptr;



INT WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //trace::DebugConsole::allocDebugConsole();

        DisableThreadLibraryCalls((HMODULE)hModule);

        wchar_t name[MAX_PATH];
        GetModuleFileNameW(hModule, name, MAX_PATH);
        ::LoadLibraryW(name);

        g_moduleHandle = hModule;

        HookApp::initialize();
        CloseHandle(CreateThread(nullptr, 0, initEventHookThread, nullptr, 0, nullptr));
    }
    if (dwReason == DLL_PROCESS_DETACH)
    {
        shutdownEventHook();
        HookApp::uninitialize();
    }

    return TRUE;
}

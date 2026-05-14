#pragma once

// Callback type for firing game events to the overlay IPC system.
using GameEventCallback = void (*)(const char* eventName, const char* eventData);
extern GameEventCallback g_onGameEvent;

// Initialize CS2 game event hooking.
bool initEventHook();

// Clean up.
void shutdownEventHook();

// Returns a status string: "ok" if init succeeded, or error description.
const char* getEventHookStatus();

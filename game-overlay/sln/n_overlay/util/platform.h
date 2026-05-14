#pragma once

// n_overlay adaptation: we're an injected DLL, not a game plugin.
// No __declspec(dllimport) — resolve game globals at runtime via GetProcAddress.

#define PLATFORM_WINDOWS

#define MS_DECL_ALIGN(x) __declspec(align(x))
#define MS_ALIGN4 MS_DECL_ALIGN(4)
#define MS_ALIGN8 MS_DECL_ALIGN(8)
#define MS_ALIGN16 MS_DECL_ALIGN(16)
#define MS_ALIGN32 MS_DECL_ALIGN(32)
#define MS_ALIGN128 MS_DECL_ALIGN(128)
#define MS_ALIGN4_POST
#define MS_ALIGN8_POST
#define MS_ALIGN16_POST
#define MS_ALIGN32_POST
#define MS_ALIGN128_POST

#define MS_IMPORT
#define MS_CLASS_IMPORT
#define MS_GLOBAL_IMPORT
#define MS_FMTFUNCTION(a, b)
#define MS_EXPORT
#define MS_EXPORT_C_API

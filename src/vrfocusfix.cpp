// VRFocusFix - openvr_api.dll proxy that fixes the reversed Steam-menu pause
// in DiRT Rally 2.0 on Steam Frame.
//
// The game stops rendering when SteamVR reports "dashboard deactivated" (503)
// and resumes on "dashboard activated" (502) - the opposite of what those
// events mean. This DLL sits between the game and the real openvr_api.dll and
// swaps those two events, so the game pauses while the Steam menu is open and
// resumes as soon as it closes. Everything else is passed through untouched.
//
// Install: rename the game's openvr_api.dll to openvr_api_orig.dll and put this
// DLL in its place. Settings: vrfocusfix.ini next to the DLL (optional).
//
// SPDX-License-Identifier: MIT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define VRFF_VERSION "1.0.0"

// ---------------------------------------------------------------- config ---

enum EventFix { EF_NONE = 0, EF_SWAP, EF_DROP };

static struct {
    EventFix eventFix = EF_SWAP;
    bool log = false;
} g_cfg;

static wchar_t g_dir[MAX_PATH];
static CRITICAL_SECTION g_logLock, g_hookLock;
static FILE* g_log = nullptr;

static void Log(const char* fmt, ...)
{
    if (!g_cfg.log) return;
    EnterCriticalSection(&g_logLock);
    if (!g_log) {
        wchar_t p[MAX_PATH];
        _snwprintf(p, MAX_PATH, L"%ls\\vrfocusfix.log", g_dir);
        g_log = _wfopen(p, L"w");
    }
    if (g_log) {
        SYSTEMTIME t; GetLocalTime(&t);
        fprintf(g_log, "[%02d:%02d:%02d.%03d] ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
        va_list a; va_start(a, fmt); vfprintf(g_log, fmt, a); va_end(a);
        fputc('\n', g_log);
        fflush(g_log);
    }
    LeaveCriticalSection(&g_logLock);
}

static void LoadConfig()
{
    wchar_t ini[MAX_PATH], buf[64];
    _snwprintf(ini, MAX_PATH, L"%ls\\vrfocusfix.ini", g_dir);
    GetPrivateProfileStringW(L"VRFocusFix", L"EventFix", L"swap", buf, 64, ini);
    if (!_wcsicmp(buf, L"none") || !_wcsicmp(buf, L"off")) g_cfg.eventFix = EF_NONE;
    else if (!_wcsicmp(buf, L"drop")) g_cfg.eventFix = EF_DROP;
    else g_cfg.eventFix = EF_SWAP;
    g_cfg.log = GetPrivateProfileIntW(L"VRFocusFix", L"Log", 0, ini) != 0;
}

// ------------------------------------------------------------- events ---
// VREvent_t starts with uint32 eventType, uint32 trackedDeviceIndex.

enum : uint32_t { EV_DASHBOARD_ACTIVATED = 502, EV_DASHBOARD_DEACTIVATED = 503 };

// Returns true if the event must be hidden from the game.
static bool ProcessEvent(void* ev)
{
    uint32_t& t = *(uint32_t*)ev;
    if (t != EV_DASHBOARD_ACTIVATED && t != EV_DASHBOARD_DEACTIVATED) return false;
    const char* name = t == EV_DASHBOARD_ACTIVATED ? "DashboardActivated" : "DashboardDeactivated";
    switch (g_cfg.eventFix) {
        case EF_SWAP:
            t = t == EV_DASHBOARD_ACTIVATED ? EV_DASHBOARD_DEACTIVATED : EV_DASHBOARD_ACTIVATED;
            Log("SteamVR sent %s, game sees %s", name,
                t == EV_DASHBOARD_ACTIVATED ? "DashboardActivated" : "DashboardDeactivated");
            return false;
        case EF_DROP:
            Log("SteamVR sent %s, hidden from game", name);
            return true;
        default:
            Log("SteamVR sent %s, passed through", name);
            return false;
    }
}

// --------------------------------------------------- PollNextEvent hooks ---
// vtable slots of PollNextEvent / PollNextEventWithPose for every published
// IVRSystem version that has a size argument (ValveSoftware/openvr headers).

struct SysSlots { int ver, poll, pollPose; };
static const SysSlots kSys[] = {
    {11, 28, 29}, {12, 28, 29}, {14, 28, 29}, {15, 28, 29}, {16, 29, 30}, {17, 29, 30},
    {19, 30, 31}, {20, 30, 31}, {21, 30, 31}, {22, 29, 30}, {23, 29, 30}, {26, 30, 31},
};

typedef bool (*PFN_Poll)(void*, void*, uint32_t);
typedef bool (*PFN_PollPose)(void*, int, void*, uint32_t, void*);
typedef bool (*PFN_FPoll)(void*, uint32_t);
typedef bool (*PFN_FPollPose)(int, void*, uint32_t, void*);

// C++ interfaces: the runtime's vtable is patched in place. The original is
// looked up again from `this`, so several interface versions can coexist.
struct VHook { void** vtbl; int pose; void* orig; };
static VHook g_vhooks[32];
static volatile LONG g_vhookCount = 0;
static void* g_fnPoll = nullptr;       // FnTable (C API) originals
static void* g_fnPollPose = nullptr;

static void* FindOrig(void* self, int pose)
{
    void** vtbl = *(void***)self;
    LONG n = g_vhookCount;
    for (LONG i = 0; i < n; i++)
        if (g_vhooks[i].vtbl == vtbl && g_vhooks[i].pose == pose) return g_vhooks[i].orig;
    return nullptr;
}

static bool H_Poll(void* s, void* ev, uint32_t sz)
{
    PFN_Poll o = (PFN_Poll)FindOrig(s, 0);
    for (;;) { if (!o(s, ev, sz)) return false; if (!ProcessEvent(ev)) return true; }
}
static bool H_PollPose(void* s, int origin, void* ev, uint32_t sz, void* pose)
{
    PFN_PollPose o = (PFN_PollPose)FindOrig(s, 1);
    for (;;) { if (!o(s, origin, ev, sz, pose)) return false; if (!ProcessEvent(ev)) return true; }
}
static bool F_Poll(void* ev, uint32_t sz)
{
    for (;;) { if (!((PFN_FPoll)g_fnPoll)(ev, sz)) return false; if (!ProcessEvent(ev)) return true; }
}
static bool F_PollPose(int origin, void* ev, uint32_t sz, void* pose)
{
    for (;;) { if (!((PFN_FPollPose)g_fnPollPose)(origin, ev, sz, pose)) return false; if (!ProcessEvent(ev)) return true; }
}

static bool WriteSlot(void** slot, void* value)
{
    DWORD old;
    if (!VirtualProtect(slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &old)) return false;
    InterlockedExchangePointer(slot, value);
    VirtualProtect(slot, sizeof(void*), old, &old);
    return true;
}

// The original pointer is recorded before the slot is overwritten, so a
// thread that lands in our function can always find it.
static void HookVtbl(void* iface, int slot, int pose, void* repl)
{
    EnterCriticalSection(&g_hookLock);
    void** vtbl = *(void***)iface;
    bool done = false;
    for (LONG i = 0; i < g_vhookCount; i++)
        if (g_vhooks[i].vtbl == vtbl && g_vhooks[i].pose == pose) done = true;
    if (!done && vtbl[slot] != repl && g_vhookCount < 32) {
        g_vhooks[g_vhookCount] = { vtbl, pose, vtbl[slot] };
        MemoryBarrier();
        InterlockedIncrement(&g_vhookCount);
        WriteSlot(&vtbl[slot], repl);
    }
    LeaveCriticalSection(&g_hookLock);
}

static void HookFnTable(void* table, int slot, void** origOut, void* repl)
{
    EnterCriticalSection(&g_hookLock);
    void** p = &((void**)table)[slot];
    if (*p != repl) { *origOut = *p; MemoryBarrier(); WriteSlot(p, repl); }
    LeaveCriticalSection(&g_hookLock);
}

static void InstallHooks(const char* version, void* iface)
{
    bool fn = strncmp(version, "FnTable:", 8) == 0;
    const char* name = fn ? version + 8 : version;
    if (strncmp(name, "IVRSystem_", 10) != 0) return;
    int v = atoi(name + 10);
    for (const SysSlots& s : kSys) {
        if (s.ver != v) continue;
        if (fn) {
            HookFnTable(iface, s.poll, &g_fnPoll, (void*)F_Poll);
            HookFnTable(iface, s.pollPose, &g_fnPollPose, (void*)F_PollPose);
        } else {
            HookVtbl(iface, s.poll, 0, (void*)H_Poll);
            HookVtbl(iface, s.pollPose, 1, (void*)H_PollPose);
        }
        Log("hooked event polling of %s", version);
        return;
    }
    Log("WARNING: %s is an unknown IVRSystem version, events not changed", version);
}

// -------------------------------------------------- original DLL loading ---

#include "thunks.h"

typedef void* (*PFN_GetGenericInterface)(const char*, int*);
static PFN_GetGenericInterface g_realGGI = nullptr;
static HMODULE g_orig = nullptr;
static void* g_exports[VRFF_EXPORT_COUNT];
static INIT_ONCE g_origOnce = INIT_ONCE_STATIC_INIT;

static BOOL CALLBACK LoadOrigOnce(PINIT_ONCE, PVOID, PVOID*)
{
    wchar_t p[MAX_PATH];
    _snwprintf(p, MAX_PATH, L"%ls\\openvr_api_orig.dll", g_dir);
    g_orig = LoadLibraryExW(p, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!g_orig) { Log("ERROR: could not load %ls (error %lu)", p, GetLastError()); return TRUE; }
    g_realGGI = (PFN_GetGenericInterface)GetProcAddress(g_orig, "VR_GetGenericInterface");
    for (int i = 0; i < VRFF_EXPORT_COUNT; i++)
        g_exports[i] = (void*)GetProcAddress(g_orig, kExportNames[i]);
    return TRUE;
}

static bool EnsureOrig()
{
    InitOnceExecuteOnce(&g_origOnce, LoadOrigOnce, nullptr, nullptr);
    return g_orig && g_realGGI;
}

static void Fatal(const wchar_t* msg)
{
    MessageBoxW(nullptr, msg, L"VRFocusFix", MB_OK | MB_ICONERROR);
    ExitProcess(1);
}

// Called by the assembly thunks in thunks.h for every pass-through export.
extern "C" void* VRFF_Resolve(int idx)
{
    if (!EnsureOrig())
        Fatal(L"VRFocusFix could not load openvr_api_orig.dll.\n\n"
              L"It must be the game's original openvr_api.dll, renamed, in the same folder. "
              L"Run uninstall.bat or verify the game files in Steam to restore the game.");
    if (!g_exports[idx])
        Fatal(L"VRFocusFix: the game called a function its original openvr_api.dll doesn't have.");
    return g_exports[idx];
}

extern "C" __declspec(dllexport) void* VR_GetGenericInterface(const char* version, int* err)
{
    if (!EnsureOrig()) { if (err) *err = 108; return nullptr; }   // VRInitError_Init_InterfaceNotFound
    void* r = g_realGGI(version, err);
    if (r && version && g_cfg.eventFix != EF_NONE) InstallHooks(version, r);
    return r;
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(inst);
        InitializeCriticalSection(&g_logLock);
        InitializeCriticalSection(&g_hookLock);
        GetModuleFileNameW(inst, g_dir, MAX_PATH);
        if (wchar_t* slash = wcsrchr(g_dir, L'\\')) *slash = 0;
        LoadConfig();
        static const char* ef[] = { "none", "swap", "drop" };
        Log("VRFocusFix " VRFF_VERSION " loaded, EventFix=%s", ef[g_cfg.eventFix]);
    }
    return TRUE;
}

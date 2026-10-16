// Fake openvr_api_orig.dll for tests: an IVRSystem_019 whose PollNextEvent
// returns queued events. Tests push events through PushEvent().
#include <windows.h>
#include <stdint.h>
#include <string.h>
static int q[64]; static int qn = 0, qi = 0;
extern "C" __declspec(dllexport) void PushEvent(int t) { q[qn++] = t; }
static bool Dflt(void*) { return false; }
static bool Poll(void*, void* ev, uint32_t sz) {
    if (qi >= qn) return false;
    memset(ev, 0, sz); ((uint32_t*)ev)[0] = q[qi++]; return true;
}
static void* vt[64]; static void* fn[64];
struct Obj { void** vptr; }; static Obj sys{vt};
extern "C" __declspec(dllexport) void* VR_GetGenericInterface(const char* v, int* e) {
    static bool init = false;
    if (!init) { init = true; for (int i = 0; i < 64; i++) vt[i] = fn[i] = (void*)Dflt; vt[30] = (void*)Poll; }
    if (e) *e = 0;
    if (!strcmp(v, "IVRSystem_019")) return &sys;
    if (e) *e = 108; return 0;
}
extern "C" __declspec(dllexport) int VR_IsHmdPresent() { return 1; }

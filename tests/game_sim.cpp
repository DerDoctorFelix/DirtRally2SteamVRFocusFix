// Simulates DiRT Rally 2.0's observed behaviour: it stops rendering on
// DashboardDeactivated (503) and renders on DashboardActivated (502).
// SteamVR sends 503 once at startup, then 502 when the Steam menu opens and
// 503 when it closes. Prints whether the game renders while the menu is closed.
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
typedef void* (*GGI)(const char*, int*); typedef bool (*P)(void*, void*, uint32_t);
int main() {
    setvbuf(stdout, 0, _IONBF, 0);
    GGI g = (GGI)GetProcAddress(LoadLibraryA("openvr_api.dll"), "VR_GetGenericInterface");
    int e; void* s = g("IVRSystem_019", &e);
    typedef void (*PE)(int); PE push = (PE)GetProcAddress(GetModuleHandleA("openvr_api_orig.dll"), "PushEvent");
    bool rendering = true; int badFrames = 0;   // frames not rendered while menu closed
    bool menuOpen = false;
    push(503);                                   // startup
    for (int f = 0; f < 100; f++) {
        if (f == 20) { menuOpen = true;  push(502); }
        if (f == 40) { menuOpen = false; push(503); }
        if (f == 60) { menuOpen = true;  push(502); }
        if (f == 80) { menuOpen = false; push(503); }
        uint32_t ev[16];
        while (((P)(*(void***)s)[30])(s, ev, 64)) {
            if (ev[0] == 503) rendering = false;
            if (ev[0] == 502) rendering = true;
        }
        if (!menuOpen && !rendering) badFrames++;
    }
    printf("frames frozen while Steam menu closed: %d\n", badFrames);
    return badFrames ? 1 : 0;
}

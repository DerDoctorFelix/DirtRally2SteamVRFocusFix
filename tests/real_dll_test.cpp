// Statically imports openvr_api.dll like a real game and calls through the
// proxy into a real (renamed) openvr_api_orig.dll. Without SteamVR installed the
// calls fail cleanly; the point is that nothing crashes and errors match.
#include <stdio.h>
#include <stdint.h>
extern "C" __declspec(dllimport) bool VR_IsHmdPresent();
extern "C" __declspec(dllimport) bool VR_IsRuntimeInstalled();
extern "C" __declspec(dllimport) uint32_t VR_InitInternal(int* err, int type);
extern "C" __declspec(dllimport) void VR_ShutdownInternal();
extern "C" __declspec(dllimport) void* VR_GetGenericInterface(const char*, int*);
extern "C" __declspec(dllimport) const char* VR_GetVRInitErrorAsEnglishDescription(int);
int main() {
    printf("VR_IsRuntimeInstalled=%d VR_IsHmdPresent=%d\n", VR_IsRuntimeInstalled(), VR_IsHmdPresent());
    int e = -1; VR_InitInternal(&e, 1);
    printf("VR_InitInternal err=%d (%s)\n", e, VR_GetVRInitErrorAsEnglishDescription(e));
    void* s = VR_GetGenericInterface("IVRSystem_019", &e);
    printf("VR_GetGenericInterface -> %p err=%d\n", s, e);
    VR_ShutdownInternal();
    printf("clean exit\n");
    return 0;
}

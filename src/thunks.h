// Generated: pass-through thunks for every openvr_api.dll export except VR_GetGenericInterface.
// Each thunk saves the argument registers, resolves the real function from openvr_api_orig.dll
// (loaded by full path from this DLL's folder) and jumps to it, so any signature works.
#pragma once
#define VRFF_EXPORT_COUNT 24
static const char* const kExportNames[VRFF_EXPORT_COUNT] = {
  "LiquidVR",
  "VRCompositorSystemInternal",
  "VRControlPanel",
  "VRDashboardManager",
  "VROculusDirect",
  "VROverlayView",
  "VRPaths",
  "VRRenderModelsInternal",
  "VRSceneGraph",
  "VRTrackedCameraInternal",
  "VRVirtualDisplay",
  "VRHeadsetView",
  "VR_GetInitToken",
  "VR_GetStringForHmdError",
  "VR_GetVRInitErrorAsEnglishDescription",
  "VR_GetVRInitErrorAsSymbol",
  "VR_InitInternal",
  "VR_InitInternal2",
  "VR_IsHmdPresent",
  "VR_IsInterfaceVersionValid",
  "VR_IsRuntimeInstalled",
  "VR_RuntimePath",
  "VR_GetRuntimePath",
  "VR_ShutdownInternal",
};
extern "C" void* VRFF_Resolve(int idx);
#define VRFF_THUNK(name, idx) \
  __asm__(".text\n.globl thunk_" #name "\nthunk_" #name ":\n" \
    "push %rcx\npush %rdx\npush %r8\npush %r9\nsub $0x68, %rsp\n" \
    "movdqu %xmm0, 0x20(%rsp)\nmovdqu %xmm1, 0x30(%rsp)\nmovdqu %xmm2, 0x40(%rsp)\nmovdqu %xmm3, 0x50(%rsp)\n" \
    "mov $" #idx ", %ecx\ncall VRFF_Resolve\n" \
    "movdqu 0x20(%rsp), %xmm0\nmovdqu 0x30(%rsp), %xmm1\nmovdqu 0x40(%rsp), %xmm2\nmovdqu 0x50(%rsp), %xmm3\n" \
    "add $0x68, %rsp\npop %r9\npop %r8\npop %rdx\npop %rcx\njmp *%rax\n");
VRFF_THUNK(LiquidVR, 0)
VRFF_THUNK(VRCompositorSystemInternal, 1)
VRFF_THUNK(VRControlPanel, 2)
VRFF_THUNK(VRDashboardManager, 3)
VRFF_THUNK(VROculusDirect, 4)
VRFF_THUNK(VROverlayView, 5)
VRFF_THUNK(VRPaths, 6)
VRFF_THUNK(VRRenderModelsInternal, 7)
VRFF_THUNK(VRSceneGraph, 8)
VRFF_THUNK(VRTrackedCameraInternal, 9)
VRFF_THUNK(VRVirtualDisplay, 10)
VRFF_THUNK(VRHeadsetView, 11)
VRFF_THUNK(VR_GetInitToken, 12)
VRFF_THUNK(VR_GetStringForHmdError, 13)
VRFF_THUNK(VR_GetVRInitErrorAsEnglishDescription, 14)
VRFF_THUNK(VR_GetVRInitErrorAsSymbol, 15)
VRFF_THUNK(VR_InitInternal, 16)
VRFF_THUNK(VR_InitInternal2, 17)
VRFF_THUNK(VR_IsHmdPresent, 18)
VRFF_THUNK(VR_IsInterfaceVersionValid, 19)
VRFF_THUNK(VR_IsRuntimeInstalled, 20)
VRFF_THUNK(VR_RuntimePath, 21)
VRFF_THUNK(VR_GetRuntimePath, 22)
VRFF_THUNK(VR_ShutdownInternal, 23)

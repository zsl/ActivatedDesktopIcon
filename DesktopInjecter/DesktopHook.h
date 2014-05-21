#ifndef DESKTOP_HOOK_H__
#define DESKTOP_HOOK_H__

#ifdef DKHOOKAPI_EXPORT
	#define DKHOOKAPI extern "C" __declspec(dllexport)
#else
    #define DKHOOKAPI extern "C" __declspec(dllimport)
	#pragma comment(lib, "DesktopHook.lib")
#endif

#include <windows.h>

DKHOOKAPI BOOL SetDesktopHook(DWORD dwThreadId);
DKHOOKAPI HWND GetDesktopDefViewHwnd();

#endif
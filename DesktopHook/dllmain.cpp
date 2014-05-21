// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <string>
#include <algorithm>
#include <cstring>
#include <cctype>

HINSTANCE g_hinstDll        = NULL;
WNDPROC   g_oldProc         = NULL;
HWND      g_hwndDefView     = NULL;
DWORD     g_desktopThreadId = 0;

// g_hook需要DesktopInjecter和桌面进程共享
#pragma data_seg("Shared")
HHOOK g_hook         = NULL;
bool  g_shouldUnLoad = false;
DWORD g_hostThreadId  = 0;
#pragma data_seg()

#pragma comment(linker, "/section:Shared,rws")


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        g_hinstDll = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


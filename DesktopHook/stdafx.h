// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
#include <cassert>
#include <tchar.h>
#include "DesktopHook.h"
#include "dbgprint.h"


extern HINSTANCE g_hinstDll;        // 我们自己的进程中使用
extern DWORD     g_desktopThreadId; //

extern HHOOK     g_hook;         // 桌面进程和我们的进程共享
extern bool      g_shouldUnLoad; // 桌面进程和我们的进程共享
extern DWORD     g_hostThreadId;  // 桌面进程和我们的进程共享

extern WNDPROC   g_oldProc;      // 桌面进程中使用，桌面ListView的旧消息处理函数
extern HWND      g_hwndDefView;       // 桌面进程中使用，桌面ListView的句柄
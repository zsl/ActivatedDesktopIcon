#include "DesktopHook.h"

#include <iostream>

int main()
{
    HWND desktopLVHwnd = GetDesktopDefViewHwnd();
    if (0 == desktopLVHwnd)
    {
        std::cerr << "error: GetDesktopLVHwnd";
        return 1;
    }

    BOOL ret = SetDesktopHook(GetWindowThreadProcessId(desktopLVHwnd, NULL));
    if (ret != TRUE)
    {
        std::cerr << "error: SetDesktopHook\n";
    }

    std::cout << "按任意键退出...";
    getchar();

    // 退出时必须先取消Hook，否则会崩溃的！
    SetDesktopHook(0);
}
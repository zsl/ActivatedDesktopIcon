#include "stdafx.h"
#include <CommCtrl.h>

#include <array>

LRESULT APIENTRY DesktopDefViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool bHandled = false;
    LRESULT ret = 0;

    switch (uMsg)
    {
    case WM_NOTIFY:
        {
            NMHDR *nmHdr = reinterpret_cast<NMHDR*>(lParam);
            assert(nmHdr);

            if (!nmHdr) break;

            switch (nmHdr->code)
            {
            case NM_CLICK:
                {
                    DbgPrint(_T("hook NM_CLICK"));
                    TCHAR text[MAX_PATH];
                    ::memset(text, 0, sizeof text);
                    NMITEMACTIVATE *nmInfo = reinterpret_cast<NMITEMACTIVATE*>(lParam);
                    ListView_GetItemText(nmHdr->hwndFrom, nmInfo->iItem, nmInfo->iSubItem, text, MAX_PATH);
                    DbgPrint(_T("NM_CLICK, GetItemText:%s"), text);
                    if (0 == _tcsicmp(text, _T("zsl.activateicon")))
                    {
                        bHandled = true;
                        ::MessageBox(NULL, text, _T("DesktopHook"), MB_OK );
                    }
                }
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

    if (bHandled)
    {
        return ret;
    }

    if (!g_oldProc) DbgPrint(_T("SubclassProc: oldProc null"));

    return ::CallWindowProc(g_oldProc, hwnd, uMsg, wParam, lParam);
}
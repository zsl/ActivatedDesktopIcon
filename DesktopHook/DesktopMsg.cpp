#include "stdafx.h"
#include <CommCtrl.h>
#include <gdiplus.h>

#include <array>

#pragma comment(lib, "gdiplus.lib")

LRESULT OnLVClick(NMITEMACTIVATE* nmInfo, bool &bHandled);
LRESULT OnLVCustomDraw(NMLVCUSTOMDRAW* nmInfo, bool &bHandled);

// 定制DefView的事件
LRESULT APIENTRY DesktopDefViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool bHandled = false;
    LRESULT ret = 0;

    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            NMHDR* nmHdr = reinterpret_cast<NMHDR*>(lParam);
            assert(nmHdr);

            if (!nmHdr) break;

            switch (nmHdr->code)
            {
                case NM_CLICK:
                {
                    DbgPrint(_T("hook NM_CLICK"));
                    NMITEMACTIVATE* nmInfo = reinterpret_cast<NMITEMACTIVATE*>(lParam);
                    ret = OnLVClick(nmInfo, bHandled);
                    break;
                }
                case NM_CUSTOMDRAW:
                {
                    NMLVCUSTOMDRAW* nmInfo = reinterpret_cast<NMLVCUSTOMDRAW*>(lParam);
                    ret = OnLVCustomDraw(nmInfo, bHandled);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    if (bHandled)
    {
        return ret;
    }

    if (!g_oldDefViewProc) DbgPrint(_T("SubclassProc: oldProc null"));

    return ::CallWindowProc(g_oldDefViewProc, hwnd, uMsg, wParam, lParam);
}

LRESULT OnLVClick(NMITEMACTIVATE* nmInfo, bool &bHandled)
{
    TCHAR text[MAX_PATH];
    ::memset(text, 0, sizeof text);

    ListView_GetItemText(nmInfo->hdr.hwndFrom, nmInfo->iItem, nmInfo->iSubItem, text, MAX_PATH);

    DbgPrint(_T("NM_CLICK, GetItemText:%s"), text);

    if (0 == _tcsicmp(text, _T("zsl.activateicon")))
    {
        bHandled = true;
        ::MessageBox(NULL, text, _T("DesktopHook"), MB_OK );
    }

    return 0;
}

LRESULT OnLVCustomDraw(NMLVCUSTOMDRAW* nmInfo, bool &bHandled)
{
    LRESULT ret = 0;
    bHandled = true;

    switch (nmInfo->nmcd.dwDrawStage)
    {
        case CDDS_PREPAINT:
        {
            ret = CDRF_NOTIFYITEMDRAW;
            break;
            TCHAR text[MAX_PATH];
            ::memset(text, 0, sizeof text);

            int itemIndex = static_cast<int>(nmInfo->nmcd.dwItemSpec);
            ListView_GetItemText(nmInfo->nmcd.hdr.hwndFrom, itemIndex, nmInfo->iSubItem, text, MAX_PATH);

            DbgPrint(_T("CustomDraw, GetItemText:%s"), text);

            if (0 == _tcsicmp(text, _T("zsl.activateicon")))
            {
                DbgPrint(_T("LVCustomDraw PREPAINT"));
                ret = CDRF_NOTIFYITEMDRAW;
            }
            else
            {
                ret = CDRF_DODEFAULT;
            }
            break;
        }
        case CDDS_ITEMPREPAINT:
        {
            DbgPrint(_T("LVCustomDraw ItemPrepaint"));
            ret = CDRF_NOTIFYPOSTPAINT;
            break;
        }
        case CDDS_ITEMPOSTPAINT:
        {
            TCHAR text[MAX_PATH];
            ::memset(text, 0, sizeof text);

            int itemIndex = static_cast<int>(nmInfo->nmcd.dwItemSpec);
            ListView_GetItemText(nmInfo->nmcd.hdr.hwndFrom, itemIndex, nmInfo->iSubItem, text, MAX_PATH);

            DbgPrint(_T("LVCustomDraw ItemPostPaint, GetItemText:%s"), text);

            if (0 ==_tcsicmp(text, _T("zsl.activateicon")))
            {
                // 在这里画我们自己的图标
                int itemIndex = static_cast<int>(nmInfo->nmcd.dwItemSpec);

                RECT iconRect;
                ListView_GetItemRect(nmInfo->nmcd.hdr.hwndFrom, itemIndex, &iconRect, LVIR_ICON);

                Gdiplus::Graphics g(nmInfo->nmcd.hdc);
                Gdiplus::Rect paintRect(iconRect.left, iconRect.top, 
                                     iconRect.right - iconRect.left, iconRect.bottom - iconRect.top
                                );
                Gdiplus::Image iconImg(L"d:\\zsl.jpg");
                g.DrawImage(&iconImg, paintRect);
       
                ret = CDRF_SKIPDEFAULT;
                break;
            }
            else
            {
                bHandled = false;
            }
        }
    }
    return ret;
}
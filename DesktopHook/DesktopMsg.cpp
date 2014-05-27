#include "stdafx.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <shlwapi.h>

#include <array>
#include <vector>
#include <memory>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")

class ImageInfo
{
public:
    ImageInfo() : m_frameCount(0), m_curFrame(0) {}

    UINT GetFrameCount() { assert(m_img); return m_frameCount; }

    UINT GetCurrentFrame() { assert(m_img); return m_curFrame; }

    void SetCurrentFrame(UINT index)
    { 
        assert(m_img);  
        assert(index < m_frameCount);

        m_curFrame = index;
        m_img->SelectActiveFrame(&Gdiplus::FrameDimensionTime, index);
    }

    long GetFrameDuration(UINT frame)
    {
        assert(m_img);
        assert(frame < m_frameCount);

        return static_cast<long*>(m_propertyItem->value)[frame] * 10;
    }

    void SetImage(const std::wstring &strImgFile)
    {
        m_strImgFile = strImgFile;
        m_img = std::make_shared<Gdiplus::Bitmap>(m_strImgFile.c_str());

        UINT frameDimensionCount = m_img->GetFrameDimensionsCount();
        if (frameDimensionCount > 0)
        {
            // 获取Frame数
            std::vector<GUID> dimensionId(frameDimensionCount);
            m_img->GetFrameDimensionsList(dimensionId.data(), frameDimensionCount);
            m_frameCount = m_img->GetFrameCount(&dimensionId[0]);
            m_curFrame = 0;

            // 获取Frame的时长
            UINT frameDelaySize = m_img->GetPropertyItemSize(PropertyTagFrameDelay);
            m_propertyItem = std::shared_ptr<Gdiplus::PropertyItem>(
                                                   (Gdiplus::PropertyItem*)malloc(frameDelaySize),
                                                   free
                                               );

            m_img->GetPropertyItem(PropertyTagFrameDelay, frameDelaySize, m_propertyItem.get());
        }
        else
        {
            m_frameCount = 0;
            m_curFrame = 0;
        }
    }

    HBITMAP GetHBITMAP()
    {
        assert(m_img);

        HBITMAP result = 0;
        m_img->GetHBITMAP(0, &result);

        return result;
    }

    void Draw(Gdiplus::Graphics &g, Gdiplus::Rect &paintRect)
    {
        g.DrawImage(m_img.get(), paintRect);
    }

private:
    std::wstring m_strImgFile;
    std::shared_ptr<Gdiplus::Bitmap> m_img;
    std::shared_ptr<Gdiplus::PropertyItem> m_propertyItem;
    UINT m_frameCount;
    UINT m_curFrame;
};

class DragInfo
{
public:
    DragInfo(HIMAGELIST imglist)
        : m_imageList(imglist)
        , m_isDrag(false) 
        , m_hwnd(NULL)
    {}

    ~DragInfo(){ };

public:
    void DragEnter(HWND hwnd, POINT pt)
    {
        m_hwnd = hwnd;
        ImageList_DragEnter(hwnd, pt.x, pt.y);
    }

    void DragMove(POINT pt)
    {
        ImageList_DragMove(pt.x, pt.y);
    }

    void DragLeave()
    {
        ImageList_DragLeave(m_hwnd);
        m_hwnd = NULL;
        m_isDrag = false;
    }

    HIMAGELIST GetHandle() { return m_imageList; }

private:
    HIMAGELIST m_imageList;
    HWND m_hwnd;
    bool m_isDrag;
};

std::shared_ptr<ImageInfo> GetIconImage()
{
    static std::shared_ptr<ImageInfo> s_img;
    if (!s_img)
    {
        s_img = std::make_shared<ImageInfo>();
        // 获取图像的路径
        std::array<WCHAR, MAX_PATH> path;
        ::GetModuleFileNameW(g_hinstDll, path.data(), path.size());
        ::PathRemoveFileSpecW(path.data());
        ::PathAppendW(path.data(), L"zsl.gif");

        DbgPrint(_T("InitImage, %s"), path.data());

        assert(::PathFileExistsW(path.data()));

        s_img->SetImage(path.data());
    }

    return s_img;
}

std::shared_ptr<DragInfo> g_dragInfo;

#define TIMER_ID_REPAINTITEM 10000


LRESULT OnLVClick(NMITEMACTIVATE* nmInfo, bool &bHandled);
LRESULT OnLVCustomDraw(NMLVCUSTOMDRAW* nmInfo, bool &bHandled);
LRESULT OnRepaintItem(HWND hwnd, bool &bHandled);

LRESULT OnDragBegin(HWND hwnd, NMLISTVIEW* nmInfo, bool &bHandled);
LRESULT OnDragMove(HWND hwnd, NMLISTVIEW* nmInfo, bool &bHandled);
LRESULT OnDragEnd(HWND hwnd, NMLISTVIEW* nmInfo, bool &bHandled);

bool isPointInSpeciedItem(HWND listViewHwnd, POINT pt, RECT *rcBound = nullptr);

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
                case LVN_BEGINDRAG:
                {
                    NMLISTVIEW* nmInfo = reinterpret_cast<NMLISTVIEW*>(lParam);
                    ret = OnDragBegin(hwnd, nmInfo, bHandled);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            if (g_dragInfo)
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                g_dragInfo->DragMove(pt);

                bHandled = true;
            }

            break;
        }
        case WM_LBUTTONUP:
        {
            if (g_dragInfo)
            {
                bHandled = true;
                ReleaseCapture();

                g_dragInfo->DragLeave();
                ImageList_EndDrag();
                g_dragInfo.reset();
            }

            break;
        }
        case WM_TIMER:
        {
            switch (wParam)
            {
                case TIMER_ID_REPAINTITEM:
                {
                    ret = OnRepaintItem(hwnd, bHandled);
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
    TCHAR text[MAX_PATH] = {0};

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
        }
        case CDDS_ITEMPREPAINT:
        {
            DbgPrint(_T("LVCustomDraw ItemPrepaint"));
            ret = CDRF_NOTIFYPOSTPAINT;
            break;
        }
        case CDDS_ITEMPOSTPAINT:
        {
            TCHAR text[MAX_PATH] = {0};

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

                GetIconImage()->Draw(g, paintRect);
       
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


LRESULT APIENTRY DesktopLVSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool bHandled = false;
    LRESULT ret = 0;

    if (bHandled)
    {
        return ret;
    }

    switch (uMsg)
    {
        case WM_MOUSEMOVE:
        {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (isPointInSpeciedItem(g_hwndLV, pt))
            {
                ::SetTimer(g_hwndDefView, TIMER_ID_REPAINTITEM, 100, nullptr);
            }

            break;
        }
    }

    return CallWindowProc(g_oldLVProc, hwnd, uMsg, wParam, lParam);
}

LRESULT OnRepaintItem(HWND hwnd, bool &bHandled)
{
    bHandled = true;

    std::shared_ptr<ImageInfo> img = GetIconImage();

    // 如果只有一帧，就没有必要用timer了
    if (img->GetFrameCount() < 2)
    {
        ::KillTimer(hwnd, TIMER_ID_REPAINTITEM);
        return 0;
    }

    POINT cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(g_hwndLV, &cursorPos);

    RECT rcBound = {0};
    if (isPointInSpeciedItem(g_hwndLV, cursorPos, &rcBound))
    {
        UINT curFrame = img->GetCurrentFrame();
        curFrame++;

        if (curFrame >= img->GetFrameCount()) curFrame = 0;

        img->SetCurrentFrame(curFrame);
        ::InvalidateRect(g_hwndLV, &rcBound, FALSE);

        long curDuration = img->GetFrameDuration(curFrame);

        SetTimer(hwnd, TIMER_ID_REPAINTITEM, curDuration, nullptr);
    }
    else
    {
        // fix it : KillTimer后，gif没有显示第一帧图片
        DbgPrint(_T("Kill Timer RepaintItem"));

        ::KillTimer(hwnd, TIMER_ID_REPAINTITEM);

        img->SetCurrentFrame(0);
        ::InvalidateRect(g_hwndLV, &rcBound, FALSE);
    }

    return 0;
}

bool isPointInSpeciedItem(HWND listViewHwnd, POINT pt, RECT *rcBound/* = nullptr*/)
{
    LVHITTESTINFO hitInfo = {0};

    hitInfo.flags = LVHT_ONITEM;
    hitInfo.pt.x = pt.x;
    hitInfo.pt.y = pt.y;

    const int itemIndex = ListView_HitTest(listViewHwnd, &hitInfo);

    if (itemIndex != -1)
    {
        TCHAR text[MAX_PATH] = {0};
        ListView_GetItemText(listViewHwnd, itemIndex, hitInfo.iSubItem, text, MAX_PATH);

        DbgPrint(_T("HistTest, GetItemText:%s"), text);

        if (0 == _tcsicmp(text, _T("zsl.activateicon")))
        {
            if (rcBound)
            {
                ListView_GetItemRect(listViewHwnd, itemIndex, rcBound, LVIR_BOUNDS);
            }

            return true;
        }
    }

    return false;
}

LRESULT OnDragBegin(HWND hwnd, NMLISTVIEW* nmInfo, bool &bHandled)
{
    bHandled = false;

    const int itemIndex = nmInfo->iItem;

    if (itemIndex != -1)
    {
        HWND hLV = nmInfo->hdr.hwndFrom;

        TCHAR text[MAX_PATH] = {0};
        ListView_GetItemText(hLV, itemIndex, nmInfo->iSubItem, text, MAX_PATH);

        DbgPrint(_T("HistTest, GetItemText:%s"), text);

        // Fix It: 拖拽时，拖拽背景没有显示指定的图标
        if (0 == _tcsicmp(text, _T("zsl.activateicon")))
        {
            bHandled = true;

            POINT pt = { -10, -10 };
            g_dragInfo = std::make_shared<DragInfo>(ListView_CreateDragImage(hLV, itemIndex, &pt));

            HBITMAP bmp = GetIconImage()->GetHBITMAP();
            ImageList_Replace(g_dragInfo->GetHandle(), 0, bmp, bmp);

            ImageList_BeginDrag(g_dragInfo->GetHandle(), 0, 0, 0);
            g_dragInfo->DragEnter(hwnd, nmInfo->ptAction);

            ::SetCapture(hwnd);
        }
    }

    return 0;
}

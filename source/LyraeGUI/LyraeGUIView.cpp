
// LyraeGUIView.cpp : implementation of the CLyraeGUIView class
//

#include "stdafx.h"
#include "LyraeGUI.h"

#include "LyraeGUIDoc.h"
#include "LyraeGUIView.h"
#include "MainFrm.h"
#include <afxwin.h>
#include <atlimage.h>
#include <atlpath.h>
#include <Psapi.h>

#pragma comment(lib, "psapi.lib")

#ifdef _DEBUG
#ifdef new
#undef new
#endif
#define new DEBUG_NEW
#endif

#include "../LyraeLib/Core/LyraeFX.h"
#include "../LyraeLib/Core/Error.h"
#include "../LyraeLib/Config/EngineConfig.h"


#define GUI_UPDATE_TIMER_ID         1
#define INFO_UPDATE_TIMER_ID        2
#define STATUSBAR_TIMER_ID          3
using namespace LyraeFX;

// CLyraeGUIView

IMPLEMENT_DYNCREATE(CLyraeGUIView, CScrollView)

BEGIN_MESSAGE_MAP(CLyraeGUIView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CLyraeGUIView::OnFilePrintPreview)
    ON_WM_CREATE()
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
//    ON_COMMAND(ID_APP_TEST, &CLyraeGUIView::OnAppTest)
    ON_COMMAND(ID_FILE_OPEN, &CLyraeGUIView::OnFileOpen)
    ON_COMMAND(ID_RENDER_RENDER, &CLyraeGUIView::OnRenderRender)
    ON_COMMAND(ID_FILE_SAVE, &CLyraeGUIView::OnFileSave)
    ON_COMMAND(ID_RENDER_TERMINATE, &CLyraeGUIView::OnRenderTerminate)
    ON_COMMAND(ID_RENDER_RELOADSCENE, &CLyraeGUIView::OnRenderReloadscene)
END_MESSAGE_MAP()

// CLyraeGUIView construction/destruction

CLyraeGUIView::CLyraeGUIView() : bh(NULL), renderTarget(NULL), buffer(NULL), renderComplete(true)
{
	// TODO: add construction code here
    CSize size;
    size.cx = size.cy = 0;
    SetScrollSizes(MM_TEXT, size);
}

CLyraeGUIView::~CLyraeGUIView()
{
    if ( renderTarget ) delete renderTarget;
}



void CLyraeGUIView::InitRayTrace()
{
    using namespace LyraeFX;

    const int32_t w = gEngineConfig.GetInt32("TargetWidth");
    const int32_t h = gEngineConfig.GetInt32("TargetHeight");
    for ( int c = 0; c < sizeof(BITMAPINFOHEADER) + 16; c++ )
    {
        bitmapbuffer[c] = 0;
    }
    bh = (BITMAPINFO *) bitmapbuffer;
    bh->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bh->bmiHeader.biPlanes = 1;
    bh->bmiHeader.biBitCount = 32;
    bh->bmiHeader.biCompression = BI_BITFIELDS;
    bh->bmiHeader.biWidth = w;
    bh->bmiHeader.biHeight = h;
    ((unsigned long *)bh->bmiColors)[0] = 255 << 16;
    ((unsigned long *)bh->bmiColors)[1] = 255 << 8;
    ((unsigned long *)bh->bmiColors)[2] = 255;

    if ( renderTarget ) delete renderTarget;
    renderTarget = new RGBRenderTarget(w, h);
    renderTarget->Clear(0);
    
    renderComplete = false;
    buffer = renderTarget->mBuffer;
    //tracer->SetTarget(hdrRenderTarget->mBuffer, gEngineConfig.GetInt32("TargetWidth"), gEngineConfig.GetInt32("TargetHeight"));

}

BOOL CLyraeGUIView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH) ::GetStockObject(GRAY_BRUSH), 0);

	return CScrollView::PreCreateWindow(cs);
}

// CLyraeGUIView drawing

void CLyraeGUIView::OnDraw(CDC* pDC)
{
	CLyraeGUIDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
    int srcWidth = gEngineConfig.GetInt32("TargetWidth");
    int srcHeight = gEngineConfig.GetInt32("TargetHeight");
    int destWidth = srcWidth;
    int destHeight = srcHeight;

    StretchDIBits(pDC->m_hDC, 0, 0, destWidth, destHeight, 0, 0, 
        srcWidth, srcHeight, buffer, bh, DIB_RGB_COLORS, SRCCOPY);
}

void CLyraeGUIView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
    sizeTotal.cx = gEngineConfig.GetInt32("TargetWidth");
    sizeTotal.cy = gEngineConfig.GetInt32("TargetHeight");
	SetScrollSizes(MM_TEXT, sizeTotal);

    SetTimer(GUI_UPDATE_TIMER_ID, 1000, NULL);
    SetTimer(INFO_UPDATE_TIMER_ID, 30, NULL);
    SetTimer(STATUSBAR_TIMER_ID, 200, NULL);


}


// CLyraeGUIView printing


void CLyraeGUIView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CLyraeGUIView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLyraeGUIView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CLyraeGUIView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CLyraeGUIView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CLyraeGUIView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}

void CLyraeGUIView::OnTimer(UINT_PTR nIDEvent)
{
    switch ( nIDEvent )
    {
    case GUI_UPDATE_TIMER_ID:
        if ( !renderComplete ) {
            gEngine.ToRGBRenderTarget(renderTarget);
            Invalidate(FALSE);
        }
        break;
    case INFO_UPDATE_TIMER_ID:
        UpdateInfo();
        break;
    case STATUSBAR_TIMER_ID:
        UpdateStatus();
        break;
    }
}

void CLyraeGUIView::UpdateStatus()
{
    CMainFrame *pMainFrame = (CMainFrame *)AfxGetApp()->m_pMainWnd;
    CMFCStatusBar *statusBar = &pMainFrame->m_wndStatusBar;
    int renderIndex = statusBar->CommandToIndex(ID_INDICATOR_RENDER);
    int memIndex = statusBar->CommandToIndex(ID_INDICATOR_MEM);
    int pixelsIndex = statusBar->CommandToIndex(ID_INDICATOR_PIXELS);
    int etcIndex = statusBar->CommandToIndex(ID_INDICATOR_TIME);

    DWORD renderColor;
    if ( gEngine.engineBusy ) {
        renderColor = RGB(0, 0, 255);
    } else {
        if ( gEngine.engineOkay ) {
            renderColor = RGB(0, 255, 0);
        } else {
            renderColor = RGB(255, 0, 0);
        }
    }
    statusBar->SetPaneBackgroundColor(renderIndex, renderColor);
    CClientDC dc(this);

    PROCESS_MEMORY_COUNTERS_EX pmcx;
    if ( GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS) &pmcx, sizeof(pmcx)) ) {
        CString mem;
        mem.Format(L"%d MB", pmcx.PrivateUsage / 1048576);
        CSize size = dc.GetTextExtent(mem);
        statusBar->SetPaneInfo(memIndex, ID_INDICATOR_MEM, SBPS_NORMAL, size.cx);
        statusBar->SetPaneText(memIndex, mem);
    }

    CString pixels;
    uint32_t p = gEngine.renderedPixels;
    uint32_t tp = gEngineConfig.GetInt32("TargetWidth") * gEngineConfig.GetInt32("TargetHeight");
    float percentage = static_cast<float>(p * 100) / static_cast<float>(tp); 
    pixels.Format(L"%d/%d [%.2f%%]", p, tp, percentage);
    CSize s = dc.GetTextExtent(pixels);
    statusBar->SetPaneInfo(pixelsIndex, ID_INDICATOR_PIXELS, SBPS_NORMAL, s.cx);
    statusBar->SetPaneText(pixelsIndex, pixels);

    DWORD elapsedTime = timeGetTime() - startTime;
    if ( p == 0 ) return;
    float moreTime = static_cast<float>(elapsedTime) / static_cast<float>(p)
        * static_cast<float>(tp - p);
    int mm = static_cast<int>(moreTime) / 60000;
    int ss = (static_cast<int>(moreTime) / 1000) % 60;
    CString time;
    time.Format(L"ETC [%dm%ds]", mm, ss);
    s = dc.GetTextExtent(time);
    statusBar->SetPaneInfo(etcIndex, ID_INDICATOR_TIME, SBPS_NORMAL, s.cx);
    statusBar->SetPaneText(etcIndex, time);
}

void CLyraeGUIView::UpdateInfo()
{
    using namespace LyraeFX;
    vector<string> info;
    gEngine.ExtractInfo(info);
    gEngine.ClearInfo();

    CMainFrame *pMainFrame = (CMainFrame *)AfxGetApp()->m_pMainWnd;
    for ( uint32_t i = 0; i < info.size(); i++ )
    {
        CString str(info[i].c_str());
        int t = pMainFrame->m_wndOutput.m_wndOutputBuild.AddString(str);
        pMainFrame->m_wndOutput.m_wndOutputBuild.SelectString(t, str);
    }
}

// CLyraeGUIView diagnostics

#ifdef _DEBUG
void CLyraeGUIView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CLyraeGUIView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CLyraeGUIDoc* CLyraeGUIView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLyraeGUIDoc)));
	return (CLyraeGUIDoc*)m_pDocument;
}
#endif //_DEBUG


// CLyraeGUIView message handlers

int CLyraeGUIView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CScrollView::OnCreate(lpCreateStruct) == -1)
        return -1;
    

    return 0;
}

BOOL CLyraeGUIView::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default

    return CScrollView::OnEraseBkgnd(pDC);
    //return TRUE;
}


UINT GUIRenderThread(LPVOID param)
{
    CLyraeGUIView *view = (CLyraeGUIView *) param;

    bool renderOkay = LyraeFXRender();
    
    if ( renderOkay ) {
        gEngine.ToRGBRenderTarget(view->renderTarget);
        //view->UpdateGUI();
        view->Invalidate();
        Info("Render succeeded.");
    } else {
        Info("Render failed.");
    }

    view->renderComplete = true;
    CMainFrame *pFrame = (CMainFrame *) AfxGetApp()->GetMainWnd();
    pFrame->m_wndProperties.m_wndPropList.EnableWindow(TRUE);
    return 0;
}

UINT LoadThread(LPVOID param)
{
    CStringA file;
    CFileDialog dlg(TRUE, L"xml");
    if ( dlg.DoModal() == IDOK ) {
        file = dlg.GetPathName();
    } else {
        return -1;
    }

    string fileStr(file);
    
    if ( !gEngine.LoadSceneFromFile(fileStr) ) {
        return -1;
    }

    return 0;
}


void CLyraeGUIView::OnFileOpen()
{
    AfxBeginThread(LoadThread, NULL);
    CMainFrame *pFrame = (CMainFrame *) AfxGetApp()->GetMainWnd();
    if ( pFrame )
        pFrame->m_wndProperties.UpdateConfigData();
}


void CLyraeGUIView::OnRenderRender()
{
    if ( !renderComplete ) {
        MessageBox(L"Render in progress, please wait!");
        return;
    }
    if ( !gEngine.IsReady() ) {
        Severe("Engine not ready(No scene loaded?)");
        return;
    }
    
    startTime = timeGetTime();
    InitRayTrace();
    UpdateGUI();
    if ( gEngineConfig.GetBool("Verbose") ) Info("GUI initialized.");

    AfxBeginThread(GUIRenderThread, this);
}

void CLyraeGUIView::UpdateGUI()
{
    CSize size;
    size.cx = gEngineConfig.GetInt32("TargetWidth");
    size.cy = gEngineConfig.GetInt32("TargetHeight");
    SetScrollSizes(MM_TEXT, size);
}


void CLyraeGUIView::OnFileSave()
{
    
    CString file;
    CFileDialog dlg(FALSE, NULL, NULL, 6UL, L"PNG (*.png)|*.png");
    if ( dlg.DoModal() == IDOK ) {
        file = dlg.GetPathName();
    } else {
        return;
    }

    int width = gEngineConfig.GetInt32("TargetWidth");
    int height = gEngineConfig.GetInt32("TargetHeight");
    CImage image;
    image.Create(width, height, 32);

    StretchDIBits(image.GetDC(), 0, 0, width, height, 0, 0, 
        width, height, buffer, bh, DIB_RGB_COLORS, SRCCOPY);

    if ( gEngineConfig.GetBool("EnableOutputMetainfo") ) {
        CDC *imageDC = CDC::FromHandle(image.GetDC());
        CFont font;
        font.CreatePointFont(80, L"Consolas");
        imageDC->SelectObject(&font);
        CRect rect;
        imageDC->GetClipBox(&rect);
        imageDC->SetBkMode(TRANSPARENT);
        imageDC->SetTextColor(RGB(255, 0, 0));

        HKEY hKey;
        LPSTR StrKey = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
        LONG lResult = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, StrKey, 0, KEY_ALL_ACCESS, &hKey);
        CHAR cpuName[128];
        StrCpyA(cpuName, "<No CPU Name>");
        DWORD dwSize = 128, dwType = REG_SZ;
        if ( lResult == ERROR_SUCCESS ) {
            ::RegQueryValueExA(hKey, "ProcessorNameString", 0, &dwType, (BYTE *) cpuName, &dwSize);
        }
        ::RegCloseKey(hKey);
 
        CString metainfo;
        stringstream ss;
        ss << "Lyrae FX v" << LYRAEFX_MAJOR_VERSION << "." << LYRAEFX_MINOR_VERSION
            << "." << LYRAEFX_PATCH_VERSION;
        ss << " @ [" << gEngineConfig.GetInt32("TargetWidth") << "*" <<
            gEngineConfig.GetInt32("TargetHeight") << "]\n";
        ss << "CPU: " << cpuName << "\nRender time: " << gEngine.renderTime;
        ss << "ms  Threads: " << gEngineConfig.GetInt32("RenderThreads");
        ss << "\nPrimitives: " << gEngine.mScene->mShapes.size();
        ss << "\nLights: " << gEngine.mScene->mLights.size();
        if ( gEngineConfig.GetBool("EnableAdaptiveSampling") ) 
            ss << "\nAdaptive Sampling: On";
        if ( gEngineConfig.GetBool("EnablePhotonMapping")) {
            if ( gEngineConfig.GetBool("EnablePhotonIndirect") )
                ss << "\nIndirect photons: " << gEngine.photonMap.indirectCount;
            if ( gEngineConfig.GetBool("EnablePhotonCaustics") )
                ss << "\nCaustics photons: " << gEngine.photonMap.causticsCount;
        }
        if ( gEngineConfig.GetBool("EnableSoftShadow") )
            ss << "\nSoft shadow enabled";
        if ( gEngineConfig.GetBool("EnableVolumetricLighting") )
            ss << "\nVolumetric lighting enabled";
        if ( gEngineConfig.GetBool("DOPEnabled") )
            ss << "\nDepth of field enabled";
        if ( gEngineConfig.GetBool("EnablePostProcessing") )
            ss << "\nPost processing enabled";


        metainfo = ss.str().c_str();

        imageDC->DrawText(metainfo, &rect, DT_LEFT);
    }

    CPath path(file);
    if ( path.GetExtension() == "" ) {
        file += ".png";
    }
    image.Save(file/*, Gdiplus::ImageFormatPNG*/);
    image.ReleaseDC();

    Info("Image saved.");
}


void CLyraeGUIView::OnRenderTerminate()
{
    renderComplete = true;
    gEngine.Terminate();
    renderTarget->Clear(0);
}

UINT ReloadThread(LPVOID param)
{
    if ( !gEngine.mScene->ReloadScene() ) {
        return -1;
    }
    return 0;
}

void CLyraeGUIView::OnRenderReloadscene()
{
    if ( gEngine.IsBusy() ) return;
    AfxBeginThread(ReloadThread, NULL);

    CMainFrame *pFrame = (CMainFrame *) AfxGetApp()->GetMainWnd();
    if ( pFrame ) pFrame->m_wndProperties.UpdateConfigData();
}

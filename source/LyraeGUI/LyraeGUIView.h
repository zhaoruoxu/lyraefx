
// LyraeGUIView.h : interface of the CLyraeGUIView class
//


#pragma once

#include "../LyraeLib/Core/LyraeFX.h"
#include "../LyraeLib/Core/Engine.h"
#include "../LyraeLib/Core/RenderTarget.h"


class CLyraeGUIView : public CScrollView
{
protected: // create from serialization only
	CLyraeGUIView();
	DECLARE_DYNCREATE(CLyraeGUIView)

// Attributes
public:
	CLyraeGUIDoc* GetDocument() const;


public:
    char bitmapbuffer[sizeof(BITMAPINFOHEADER) + 16];
    BITMAPINFO *bh;
    void InitRayTrace();
    LyraeFX::RGBRenderTarget *renderTarget;
    LyraeFX::Pixel *buffer;
    bool renderComplete;
    DWORD startTime;

    void UpdateInfo();
    void UpdateGUI();
    void UpdateStatus();
    //LyraeFX::Engine *tracer;
    // UINT RenderThread(LPVOID pParam);


// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CLyraeGUIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
//    afx_msg void OnAppTest();
    afx_msg void OnFileOpen();
    afx_msg void OnRenderRender();
    afx_msg void OnFileSave();
    afx_msg void OnRenderTerminate();
    afx_msg void OnRenderReloadscene();
};



#ifndef _DEBUG  // debug version in LyraeGUIView.cpp
inline CLyraeGUIDoc* CLyraeGUIView::GetDocument() const
   { return reinterpret_cast<CLyraeGUIDoc*>(m_pDocument); }
#endif


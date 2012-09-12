
// LyraeGUIDoc.cpp : implementation of the CLyraeGUIDoc class
//

#include "stdafx.h"
#include "LyraeGUI.h"

#include "LyraeGUIDoc.h"

#include "../LyraeLib/Core/LyraeFX.h"
#include "../LyraeLib/Core/Engine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLyraeGUIDoc

IMPLEMENT_DYNCREATE(CLyraeGUIDoc, CDocument)

BEGIN_MESSAGE_MAP(CLyraeGUIDoc, CDocument)
END_MESSAGE_MAP()


// CLyraeGUIDoc construction/destruction

CLyraeGUIDoc::CLyraeGUIDoc()
{
	// TODO: add one-time construction code here

}

CLyraeGUIDoc::~CLyraeGUIDoc()
{
}

BOOL CLyraeGUIDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CLyraeGUIDoc serialization

void CLyraeGUIDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CLyraeGUIDoc diagnostics

#ifdef _DEBUG
void CLyraeGUIDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLyraeGUIDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CLyraeGUIDoc commands


void CLyraeGUIDoc::SetTitle(LPCTSTR lpszTitle)
{
    CString title;
    title.Format(L"Lyrae FX v%d.%d.%d", LYRAEFX_MAJOR_VERSION, 
        LYRAEFX_MINOR_VERSION, LYRAEFX_PATCH_VERSION);
    AfxGetMainWnd()->SetWindowText(title);
}

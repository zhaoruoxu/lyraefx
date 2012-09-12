
// LyraeGUI.h : main header file for the LyraeGUI application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CLyraeGUIApp:
// See LyraeGUI.cpp for the implementation of this class
//

class CLyraeGUIApp : public CWinAppEx
{
public:
	CLyraeGUIApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
//    virtual void AddToRecentFileList(LPCTSTR lpszPathName);
};

extern CLyraeGUIApp theApp;

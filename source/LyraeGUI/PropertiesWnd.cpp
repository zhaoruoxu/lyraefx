
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "LyraeGUI.h"

#include "../LyraeLib/Core/LyraeFX.h"
#include "../LyraeLib/Config/EngineConfig.h"
using namespace LyraeFX;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#ifdef new
#undef new
#endif
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
    ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, &CPropertiesWnd::OnPropertyChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	m_wndObjectCombo.GetWindowRect(&rectCombo);

	int cyCmb = rectCombo.Size().cy;
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    
	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("Failed to create Properties Combo \n");
		return -1;      // fail to create
	}

    /*
	m_wndObjectCombo.AddString(_T("Application"));
	m_wndObjectCombo.AddString(_T("Properties Window"));
	m_wndObjectCombo.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
	m_wndObjectCombo.SetCurSel(0);
    */
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* pCmdUI)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO: Add your command handler code here
    //MessageBox(_T("shit1"));
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::OnProperties2()
{
	// TODO: Add your command handler code here
    //MessageBox(_T("shit3"));
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

    CMFCPropertyGridProperty* pGroupBool = new CMFCPropertyGridProperty(_T("Boolean"));

    map<string, bool>::iterator iterBool;
    for ( iterBool = gEngineConfig.boolAttrs.begin(); iterBool != gEngineConfig.boolAttrs.end(); iterBool++ ) {
        CString name(iterBool->first.c_str());
        pGroupBool->AddSubItem(new CMFCPropertyGridProperty(name, (_variant_t) iterBool->second, name) );
    }
    m_wndPropList.AddProperty(pGroupBool);

    CMFCPropertyGridProperty *pGroupInt = new CMFCPropertyGridProperty(_T("Int32"));
    map<string, int>::iterator iterInt;
    for ( iterInt = gEngineConfig.int32Attrs.begin(); iterInt != gEngineConfig.int32Attrs.end(); iterInt++ ) {
        CString name(iterInt->first.c_str());
        pGroupInt->AddSubItem(new CMFCPropertyGridProperty(name, (_variant_t) iterInt->second, name));
    }
    m_wndPropList.AddProperty(pGroupInt);

    CMFCPropertyGridProperty *pGroupFloat = new CMFCPropertyGridProperty(_T("Float"));
    map<string, float>::iterator iterFloat;
    for ( iterFloat = gEngineConfig.floatAttrs.begin(); iterFloat != gEngineConfig.floatAttrs.end(); iterFloat++ ) {
        CString name(iterFloat->first.c_str());
        pGroupFloat->AddSubItem(new CMFCPropertyGridProperty(name, (_variant_t) iterFloat->second, name));
    }
    m_wndPropList.AddProperty(pGroupFloat);

    CMFCPropertyGridProperty *pGroupVector = new CMFCPropertyGridProperty(_T("Vector"));
    map<string, Vector>::iterator iterVector;
    for ( iterVector = gEngineConfig.vectorAttrs.begin(); iterVector != gEngineConfig.vectorAttrs.end(); iterVector++ ) {
        CString name(iterVector->first.c_str());
        CMFCPropertyGridProperty *pX = new CMFCPropertyGridProperty(L"X", (_variant_t) iterVector->second.x, L"X");
        CMFCPropertyGridProperty *pY = new CMFCPropertyGridProperty(L"Y", (_variant_t) iterVector->second.y, L"Y");
        CMFCPropertyGridProperty *pZ = new CMFCPropertyGridProperty(L"Z", (_variant_t) iterVector->second.z, L"Z");
        CMFCPropertyGridProperty *pVec = new CMFCPropertyGridProperty(name, 0, TRUE);
        pVec->AddSubItem(pX);
        pVec->AddSubItem(pY);
        pVec->AddSubItem(pZ);
        pGroupVector->AddSubItem(pVec);
    }
    m_wndPropList.AddProperty(pGroupVector);

    /*
	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("Appearance"));

	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("3D Look"), (_variant_t) false, _T("Specifies the window's font will be non-bold and controls will have a 3D border")));

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("Border"), _T("Dialog Frame"), _T("One of: None, Thin, Resizable, or Dialog Frame"));
	pProp->AddOption(_T("None"));
	pProp->AddOption(_T("Thin"));
	pProp->AddOption(_T("Resizable"));
	pProp->AddOption(_T("Dialog Frame"));
	pProp->AllowEdit(FALSE);

	pGroup1->AddSubItem(pProp);
	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("Caption"), (_variant_t) _T("About"), _T("Specifies the text that will be displayed in the window's title bar")));

	m_wndPropList.AddProperty(pGroup1);

	CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("Window Size"), 0, TRUE);

	pProp = new CMFCPropertyGridProperty(_T("Height"), (_variant_t) 250l, _T("Specifies the window's height"));
	pProp->EnableSpinControl(TRUE, 50, 300);
	pSize->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Width"), (_variant_t) 150l, _T("Specifies the window's width"));
	pProp->EnableSpinControl(TRUE, 50, 200);
	pSize->AddSubItem(pProp);

	m_wndPropList.AddProperty(pSize);

	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("Font"));

	LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);

	lstrcpy(lf.lfFaceName, _T("Arial"));

	pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(_T("Font"), lf, CF_EFFECTS | CF_SCREENFONTS, _T("Specifies the default font for the window")));
	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("Use System Font"), (_variant_t) true, _T("Specifies that the window uses MS Shell Dlg font")));

	m_wndPropList.AddProperty(pGroup2);

	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(_T("Misc"));
	pProp = new CMFCPropertyGridProperty(_T("(Name)"), _T("Application"));
	pProp->Enable(FALSE);
	pGroup3->AddSubItem(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Window Color"), RGB(210, 192, 254), NULL, _T("Specifies the default window color"));
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	pGroup3->AddSubItem(pColorProp);

	static TCHAR BASED_CODE szFilter[] = _T("Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||");
	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Icon"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("Specifies the window icon")));

	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Folder"), _T("c:\\")));

	m_wndPropList.AddProperty(pGroup3);

	CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(_T("Hierarchy"));

	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(_T("First sub-level"));
	pGroup4->AddSubItem(pGroup41);

	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(_T("Second sub-level"));
	pGroup41->AddSubItem(pGroup411);

	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 1"), (_variant_t) _T("Value 1"), _T("This is a description")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 2"), (_variant_t) _T("Value 2"), _T("This is a description")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 3"), (_variant_t) _T("Value 3"), _T("This is a description")));

	pGroup4->Expand(FALSE);
	m_wndPropList.AddProperty(pGroup4);
    */
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();

    
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}


afx_msg LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
    CMFCPropertyGridProperty *pProp = (CMFCPropertyGridProperty *) lParam;
    CMFCPropertyGridProperty *pPropParent = pProp->GetParent();    
    CString parentName(pPropParent->GetName());
    CString propName(pProp->GetName());
    if ( propName == "X" || propName == "Y" || propName == "Z" ) {
        pProp = pPropParent;
        pPropParent = pProp->GetParent();
        parentName = pPropParent->GetName();
        propName = pProp->GetName();
    }
    if ( parentName == "Boolean" ) {
        COleVariant value = pProp->GetValue();
        string name = CStringA(pProp->GetName());
        bool v = value.boolVal != 0;
        gEngineConfig.AddAttribute(name, value.boolVal != 0);
    } else if ( parentName == "Int32" ) {
        COleVariant value = pProp->GetValue();
        string name = CStringA(pProp->GetName());
        gEngineConfig.AddAttribute(name, value.intVal);
    } else if ( parentName == "Float" ) {
        COleVariant value = pProp->GetValue();
        string name = CStringA(pProp->GetName());
        gEngineConfig.AddAttribute(name, value.fltVal);
    } else if ( parentName == "Vector" ) {
        COleVariant x = pProp->GetSubItem(0)->GetValue();
        COleVariant y = pProp->GetSubItem(1)->GetValue();
        COleVariant z = pProp->GetSubItem(2)->GetValue();
        string name = CStringA(pProp->GetName());
        gEngineConfig.AddAttribute(name, Vector(x.fltVal, y.fltVal, z.fltVal));
    } else {
        MessageBox(L"Unknown category!!");
    }
    return 0;
}

void CPropertiesWnd::UpdateConfigData()
{
    int n = m_wndPropList.GetPropertyCount();
    for ( int i = 0; i < n; i++ ) {
        CMFCPropertyGridProperty *prop = m_wndPropList.GetProperty(i);
        CString propName(prop->GetName());
        int m = prop->GetSubItemsCount();
        if ( propName == "Boolean" ) {
            for ( int j = 0; j < m; j++ ) {
                CMFCPropertyGridProperty *subprob = prop->GetSubItem(j);
                string name = CStringA(subprob->GetName());
                subprob->SetValue((_variant_t) gEngineConfig.GetBool(name));
            }
        } else if ( propName == "Int32" ) {
            for ( int j = 0; j < m; j++ ) {
                CMFCPropertyGridProperty *subprob = prop->GetSubItem(j);
                string name = CStringA(subprob->GetName());
                subprob->SetValue((_variant_t) gEngineConfig.GetInt32(name));
            }
        } else if ( propName == "Float" ) {
            for ( int j = 0; j < m; j++ ) {
                CMFCPropertyGridProperty *subprop = prop->GetSubItem(j);
                string name = CStringA(subprop->GetName());
                subprop->SetValue((_variant_t) gEngineConfig.GetFloat(name));
            }
        } else if ( propName == "Vector" ) {
            for ( int j = 0; j < m; j++ ) {
                CMFCPropertyGridProperty *subprop = prop->GetSubItem(j);
                string name = CStringA(subprop->GetName());
                CMFCPropertyGridProperty *xyz = subprop->GetSubItem(0);
                xyz->SetValue((_variant_t) gEngineConfig.GetVector(name).x);
                xyz = subprop->GetSubItem(1);
                xyz->SetValue((_variant_t) gEngineConfig.GetVector(name).y);
                xyz = subprop->GetSubItem(2);
                xyz->SetValue((_variant_t) gEngineConfig.GetVector(name).z);
            }
        } else {
            MessageBox(L"Unknown category!!");
        }
    }
}
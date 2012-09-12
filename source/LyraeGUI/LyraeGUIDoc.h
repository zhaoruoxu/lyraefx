
// LyraeGUIDoc.h : interface of the CLyraeGUIDoc class
//


#pragma once


class CLyraeGUIDoc : public CDocument
{
protected: // create from serialization only
	CLyraeGUIDoc();
	DECLARE_DYNCREATE(CLyraeGUIDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CLyraeGUIDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
    virtual void SetTitle(LPCTSTR lpszTitle);
};



#pragma once
#include "afxdialogex.h"


// CFrameBuilderPage dialog

class CFrameBuilderPage : public CDialogEx
{
	DECLARE_DYNAMIC(CFrameBuilderPage)

public:
	CFrameBuilderPage(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFrameBuilderPage();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRAME_BUILDER_PAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

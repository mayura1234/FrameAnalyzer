// CFrameBuilderPage.cpp : implementation file
//

#include "pch.h"
#include "FrameAnalyzer.h"
#include "afxdialogex.h"
#include "CFrameBuilderPage.h"


// CFrameBuilderPage dialog

IMPLEMENT_DYNAMIC(CFrameBuilderPage, CDialogEx)

CFrameBuilderPage::CFrameBuilderPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FRAME_BUILDER_PAGE, pParent)
{

}

CFrameBuilderPage::~CFrameBuilderPage()
{
}

void CFrameBuilderPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFrameBuilderPage, CDialogEx)
END_MESSAGE_MAP()


// CFrameBuilderPage message handlers

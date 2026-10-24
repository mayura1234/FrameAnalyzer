#include "pch.h"
#include "framework.h"
#include "FrameBuilderPage.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFrameBuilderPage::CFrameBuilderPage(CWnd* pParent)
    : CDialogEx(IDD_FRAME_BUILDER_PAGE, pParent)
{
}

void CFrameBuilderPage::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFrameBuilderPage, CDialogEx)
END_MESSAGE_MAP()

BOOL CFrameBuilderPage::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    return TRUE;
}
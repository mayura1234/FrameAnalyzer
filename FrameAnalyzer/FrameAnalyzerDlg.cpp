

#include "pch.h"
#include "framework.h"
#include "FrameAnalyzer.h"
#include "FrameAnalyzerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFrameAnalyzerDlg dialog

CFrameAnalyzerDlg::CFrameAnalyzerDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_FRAMEANALYZER_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFrameAnalyzerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_MAIN, m_tabMain);
}

BEGIN_MESSAGE_MAP(CFrameAnalyzerDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CFrameAnalyzerDlg::OnTabSelectionChanged)
END_MESSAGE_MAP()

// CFrameAnalyzerDlg message handlers

BOOL CFrameAnalyzerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    // Add tab labels
    m_tabMain.InsertItem(0, _T("Frame Analyzer"));
    m_tabMain.InsertItem(1, _T("Frame Builder"));
    m_tabMain.SetCurSel(0);

    // Create child pages as children of the main dialog
    if (!m_analyzerPage.Create(IDD_FRAME_ANALYZER_PAGE, this))
    {
        AfxMessageBox(_T("Failed to create Frame Analyzer page."));
        return TRUE;
    }

    if (!m_builderPage.Create(IDD_FRAME_BUILDER_PAGE, this))
    {
        AfxMessageBox(_T("Failed to create Frame Builder page."));
        m_analyzerPage.DestroyWindow();
        return TRUE;
    }

    // Position pages inside the tab content area
    UpdateTabPagesLayout();

    // Place tab control on top in z-order so its header strip is not covered
    m_tabMain.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // Show first page
    m_analyzerPage.ShowWindow(SW_SHOW);
    m_builderPage.ShowWindow(SW_HIDE);

    return TRUE;
}

void CFrameAnalyzerDlg::UpdateTabPagesLayout()
{
    if (!::IsWindow(m_tabMain.GetSafeHwnd()))
        return;

    // Get the full tab control rect in screen coordinates
    CRect rect;
    m_tabMain.GetWindowRect(&rect);
    ScreenToClient(&rect);

    // Shrink rect to exclude the tab header strip
    m_tabMain.AdjustRect(FALSE, &rect);

    if (::IsWindow(m_analyzerPage.GetSafeHwnd()))
        m_analyzerPage.MoveWindow(&rect);

    if (::IsWindow(m_builderPage.GetSafeHwnd()))
        m_builderPage.MoveWindow(&rect);
}

void CFrameAnalyzerDlg::OnTabSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    const int sel = m_tabMain.GetCurSel();

    m_analyzerPage.ShowWindow(sel == 0 ? SW_SHOW : SW_HIDE);
    m_builderPage.ShowWindow(sel == 1 ? SW_SHOW : SW_HIDE);

    *pResult = 0;
}

void CFrameAnalyzerDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    UpdateTabPagesLayout();
}

void CFrameAnalyzerDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND,
            reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        const int cxIcon = GetSystemMetrics(SM_CXICON);
        const int cyIcon = GetSystemMetrics(SM_CYICON);

        CRect rect;
        GetClientRect(&rect);

        const int x = (rect.Width()  - cxIcon + 1) / 2;
        const int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

HCURSOR CFrameAnalyzerDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}
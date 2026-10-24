// FrameAnalyzerDlg.h : header file
//
#pragma once

#include "resource.h"
#include "FrameAnalyzerPage.h"
#include "FrameBuilderPage.h"

// CFrameAnalyzerDlg dialog
class CFrameAnalyzerDlg : public CDialogEx
{
// Construction
public:
    CFrameAnalyzerDlg(CWnd* pParent = nullptr);

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FRAMEANALYZER_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTabSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);

private:
    void UpdateTabPagesLayout();

    HICON           m_hIcon;
    CTabCtrl        m_tabMain;
    CFrameAnalyzerPage  m_analyzerPage;
    CFrameBuilderPage   m_builderPage;
};
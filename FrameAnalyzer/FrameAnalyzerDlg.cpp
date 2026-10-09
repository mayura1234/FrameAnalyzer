
// FrameAnalyzerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "FrameAnalyzer.h"
#include "FrameAnalyzerDlg.h"
#include "afxdialogex.h"
#include "resource.h"

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

	DDX_Text(pDX, IDC_EDIT_HEX_INPUT, m_strHexInput);
	DDX_Text(pDX, IDC_EDIT_OUTPUT, m_strOutput);
	DDX_Control(pDX, IDC_COMBO_PROTOCOL, m_cbProtocol);
}

BEGIN_MESSAGE_MAP(CFrameAnalyzerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_DECODE, &CFrameAnalyzerDlg::OnBnClickedBtnDecode)
END_MESSAGE_MAP()


// CFrameAnalyzerDlg message handlers

BOOL CFrameAnalyzerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_cbProtocol.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFrameAnalyzerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFrameAnalyzerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFrameAnalyzerDlg::OnBnClickedBtnDecode()
{
	// TODO: Add your control notification handler code here
	// 1. Pull data from the UI controls into your C++ variables
	UpdateData(TRUE);

	// 2. Safety check: Did the user actually paste anything?
	if (m_strHexInput.IsEmpty())
	{
		m_strOutput = _T("Error: Please paste a hex frame first.");
		UpdateData(FALSE);
		return;
	}

	// 3. Get the selected protocol from the Combo Box
	CString selectedProtocol;
	m_cbProtocol.GetWindowText(selectedProtocol);

	// 4. Format a temporary output to prove we grabbed everything
	CString result;
	result.Format(_T("Protocol Selected: %s\r\nRaw Input: %s"),
		selectedProtocol, m_strHexInput);

	m_strOutput = result;

	// 5. Push the result back to the multiline output box
	UpdateData(FALSE);
}


// FrameAnalyzerDlg.h : header file
//

#pragma once


// CFrameAnalyzerDlg dialog
class CFrameAnalyzerDlg : public CDialogEx
{
// Construction
public:
	CFrameAnalyzerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRAMEANALYZER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
<<<<<<< HEAD
public:
	// The wizard defined for strings
	CString m_strHexInput;
	CString m_strOutput;

	// The wizard for combo box controller 
	CComboBox m_cbProtocol;

	afx_msg void OnBnClickedBtnDecode();
=======
>>>>>>> d865bb1617f5256c0274ad28a97b4e53c4a1e13b
};

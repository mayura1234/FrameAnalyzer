
// FrameAnalyzerDlg.h : header file
//

#pragma once

#include "resource.h"
#include <vector>

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
public:
	// The wizard defined for strings
	CString m_strHexInput;
	CString m_strOutput;

	// The wizard for combo box controller 
	CComboBox m_cbProtocol;

	afx_msg void OnBnClickedBtnDecode();

private:
	std::vector<unsigned char> ParseHexInput(CString input);
	unsigned short CalculateCRC(const std::vector<unsigned char>& data, int length);

};
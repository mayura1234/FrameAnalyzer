
#pragma once

#include "resource.h"
#include <vector>

class CFrameAnalyzerPage : public CDialogEx
{
public:
    CFrameAnalyzerPage(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FRAME_ANALYZER_PAGE };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    CString m_strHexInput;
    CString m_strOutput;

    CComboBox m_cbProtocol;

    afx_msg void OnBnClickedBtnDecode();

private:
    bool TryParseHexInput(
        const CString& input,
        std::vector<unsigned char>& frameBytes,
        CString& errorMessage);

    unsigned short CalculateCRC(
        const std::vector<unsigned char>& data,
        int length);

    unsigned char CalculateLRC(
        const std::vector<unsigned char>& data,
        int length);

    void DecodeModbusRTU(
        const std::vector<unsigned char>& bytes,
        CString& result);

    void DecodeModbusASCII(
        const CString& input,
        CString& result);

    void DecodeModbusTCP(
        const std::vector<unsigned char>& bytes,
        CString& result);
};

#pragma once

#include "resource.h"

class CFrameBuilderPage : public CDialogEx
{
public:
    explicit CFrameBuilderPage(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FRAME_BUILDER_PAGE };
#endif

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;

    DECLARE_MESSAGE_MAP()

private:
    CComboBox m_comboProtocol;
    CComboBox m_comboFunction;

    CEdit m_editAddress;
    CEdit m_editStartRegister;
    CEdit m_editQuantity;

    CEdit m_editTransactionId;
    CEdit m_editProtocolId;

    CEdit m_editOutput;

    afx_msg void OnProtocolChanged();
    afx_msg void OnBnClickedGenerateFrame();
    afx_msg void OnBnClickedClearBuilder();
    afx_msg void OnBnClickedCopyFrame();

    void UpdateProtocolFields();

    void GenerateModbusRTUFC03();
    void GenerateModbusASCII();
    void GenerateModbusTCP();

    bool ParseUnsignedDecimal(
        const CString& input,
        unsigned int maxValue,
        unsigned int& value) const;

    static unsigned short CalculateModbusCRC(
        const BYTE* data,
        size_t length);
};


#include "pch.h"
#include "framework.h"
#include "FrameBuilderPage.h"
#include "afxdialogex.h"

#include <vector>
//#include <limits>

CFrameBuilderPage::CFrameBuilderPage(CWnd* pParent)
    : CDialogEx(IDD_FRAME_BUILDER_PAGE, pParent)
{
}

void CFrameBuilderPage::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_COMBO_BUILDER_PROTOCOL, m_comboProtocol);
    DDX_Control(pDX, IDC_COMBO_BUILDER_FRAME_TYPE, m_comboFunction);

    DDX_Control(pDX, IDC_EDIT_BUILDER_ADDRESS, m_editAddress);
    DDX_Control(pDX, IDC_EDIT_START_REGISTER, m_editStartRegister);
    DDX_Control(pDX, IDC_EDIT_REGISTER_QUANTITY, m_editQuantity);

    DDX_Control(pDX, IDC_EDIT_TRANSACTION_ID, m_editTransactionId);
    DDX_Control(pDX, IDC_EDIT_PROTOCOL_ID, m_editProtocolId);

    DDX_Control(pDX, IDC_EDIT_BUILDER_OUTPUT, m_editOutput);
}

BEGIN_MESSAGE_MAP(CFrameBuilderPage, CDialogEx)
    ON_CBN_SELCHANGE(
        IDC_COMBO_BUILDER_PROTOCOL,
        &CFrameBuilderPage::OnProtocolChanged)

    ON_BN_CLICKED(
        IDC_BTN_GENERATE_FRAME,
        &CFrameBuilderPage::OnBnClickedGenerateFrame)

    ON_BN_CLICKED(
        IDC_BTN_CLEAR_BUILDER,
        &CFrameBuilderPage::OnBnClickedClearBuilder)

    ON_BN_CLICKED(
        IDC_BTN_COPY_FRAME,
        &CFrameBuilderPage::OnBnClickedCopyFrame)
END_MESSAGE_MAP()

BOOL CFrameBuilderPage::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_comboProtocol.ResetContent();
    m_comboProtocol.AddString(_T("Modbus RTU"));
    m_comboProtocol.AddString(_T("Modbus ASCII"));
    m_comboProtocol.AddString(_T("Modbus TCP"));
    m_comboProtocol.SetCurSel(0);

    m_comboFunction.ResetContent();
    m_comboFunction.AddString(
        _T("03 - Read Holding Registers"));
    m_comboFunction.SetCurSel(0);

    m_editAddress.SetWindowText(_T("1"));
    m_editStartRegister.SetWindowText(_T("16"));
    m_editQuantity.SetWindowText(_T("2"));

    m_editTransactionId.SetWindowText(_T("1"));
    m_editProtocolId.SetWindowText(_T("0"));

    m_editOutput.SetReadOnly(TRUE);

    UpdateProtocolFields();

    return TRUE;
}

void CFrameBuilderPage::OnProtocolChanged()
{
    UpdateProtocolFields();

    m_editOutput.SetWindowText(_T(""));
}

void CFrameBuilderPage::UpdateProtocolFields()
{
    const int protocol = m_comboProtocol.GetCurSel();

    // Transaction ID and Protocol ID are specific to Modbus TCP.
    const BOOL isTcp = (protocol == 2);

    m_editTransactionId.EnableWindow(isTcp);
    m_editProtocolId.EnableWindow(isTcp);

    // Shared Modbus request fields.
    m_editAddress.EnableWindow(TRUE);
    m_editStartRegister.EnableWindow(TRUE);
    m_editQuantity.EnableWindow(TRUE);
    m_comboFunction.EnableWindow(TRUE);
}

void CFrameBuilderPage::OnBnClickedGenerateFrame()
{
    CString protocol;
    m_comboProtocol.GetWindowText(protocol);

    if (protocol == _T("Modbus RTU"))
        GenerateModbusRTUFC03();
    else if (protocol == _T("Modbus ASCII"))
        GenerateModbusASCII();
    else if (protocol == _T("Modbus TCP"))
        GenerateModbusTCP();
}

void CFrameBuilderPage::OnBnClickedClearBuilder()
{
    m_editAddress.SetWindowText(_T("1"));
    m_editStartRegister.SetWindowText(_T("16"));
    m_editQuantity.SetWindowText(_T("2"));

    m_editTransactionId.SetWindowText(_T("1"));
    m_editProtocolId.SetWindowText(_T("0"));

    m_editOutput.SetWindowText(_T(""));
}

bool CFrameBuilderPage::ParseUnsignedDecimal(
    const CString& input,
    unsigned int maxValue,
    unsigned int& value) const
{
    CString text = input;
    text.Trim();

    if (text.IsEmpty())
        return false;

    unsigned int result = 0;

    for (int i = 0; i < text.GetLength(); ++i)
    {
        const TCHAR ch = text[i];

        if (ch < _T('0') || ch > _T('9'))
            return false;

        const unsigned int digit =
            static_cast<unsigned int>(ch - _T('0'));

        if (result > (maxValue - digit) / 10)
            return false;

        result = result * 10 + digit;
    }

    value = result;
    return true;
}

unsigned short CFrameBuilderPage::CalculateModbusCRC(
    const BYTE* data,
    size_t length)
{
    unsigned short crc = 0xFFFF;

    for (size_t i = 0; i < length; ++i)
    {
        crc ^= data[i];

        for (int bit = 0; bit < 8; ++bit)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }

    return crc;
}

void CFrameBuilderPage::GenerateModbusRTUFC03()
{
    CString text;
    unsigned int address = 0;
    unsigned int start = 0;
    unsigned int quantity = 0;

    m_editAddress.GetWindowText(text);

    if (!ParseUnsignedDecimal(text, 247, address) || address < 1)
    {
        AfxMessageBox(_T("Unit ID must be between 1 and 247."));
        return;
    }

    m_editStartRegister.GetWindowText(text);

    if (!ParseUnsignedDecimal(text, 65535, start))
    {
        AfxMessageBox(
            _T("Starting address must be between 0 and 65535."));
        return;
    }

    m_editQuantity.GetWindowText(text);

    if (!ParseUnsignedDecimal(text, 125, quantity) || quantity < 1)
    {
        AfxMessageBox(_T("Quantity must be between 1 and 125."));
        return;
    }

    if (start + quantity - 1 > 65535)
    {
        AfxMessageBox(
            _T("Starting address plus quantity exceeds 65535."));
        return;
    }

    // Modbus RTU request:
    // Unit ID, Function, Start Hi, Start Lo, Quantity Hi, Quantity Lo

    std::vector<BYTE> frame =
    {
        static_cast<BYTE>(address),
        0x03,
        static_cast<BYTE>((start >> 8) & 0xFF),
        static_cast<BYTE>(start & 0xFF),
        static_cast<BYTE>((quantity >> 8) & 0xFF),
        static_cast<BYTE>(quantity & 0xFF)
    };

    const unsigned short crc =
        CalculateModbusCRC(frame.data(), frame.size());

    // Modbus RTU transmits the CRC low byte first.
    frame.push_back(static_cast<BYTE>(crc & 0xFF));
    frame.push_back(static_cast<BYTE>((crc >> 8) & 0xFF));

    CString hex;

    for (size_t i = 0; i < frame.size(); ++i)
    {
        CString byteText;
        byteText.Format(_T("%02X"), frame[i]);

        if (i > 0)
            hex += _T(" ");

        hex += byteText;
    }

    CString output=hex;

    m_editOutput.SetWindowText(output);
}
void CFrameBuilderPage::GenerateModbusASCII()
{
    CString text;
    unsigned int address = 0, start = 0, quantity = 0;

    m_editAddress.GetWindowText(text);
    if (!ParseUnsignedDecimal(text, 247, address) || address < 1)
        return;

    m_editStartRegister.GetWindowText(text);
    if (!ParseUnsignedDecimal(text, 65535, start))
        return;

    m_editQuantity.GetWindowText(text);
    if (!ParseUnsignedDecimal(text, 125, quantity) ||
        quantity < 1 || start + quantity - 1 > 65535)
        return;

    std::vector<BYTE> data =
    {
        static_cast<BYTE>(address),
        0x03,
        static_cast<BYTE>(start >> 8),
        static_cast<BYTE>(start & 0xFF),
        static_cast<BYTE>(quantity >> 8),
        static_cast<BYTE>(quantity & 0xFF)
    };

    BYTE sum = 0;
    for (BYTE b : data)
        sum += b;

    const BYTE lrc = static_cast<BYTE>(-sum);

    CString output = _T(":");

    for (BYTE b : data)
    {
        CString part;
        part.Format(_T("%02X"), b);
        output += part;
    }

    CString part;
    part.Format(_T("%02X\r\n"), lrc);
    output += part;

    m_editOutput.SetWindowText(output);
}
void CFrameBuilderPage::GenerateModbusTCP()
{
    CString text;
    unsigned int address = 0, start = 0, quantity = 0;
    unsigned int transactionId = 0, protocolId = 0;

    m_editAddress.GetWindowText(text);
    if (!ParseUnsignedDecimal(text, 247, address) || address < 1)
        return;

    m_editStartRegister.GetWindowText(text);
    if (!ParseUnsignedDecimal(text, 65535, start))
        return;

    m_editQuantity.GetWindowText(text);
    if (!ParseUnsignedDecimal(text, 125, quantity) ||
        quantity < 1 || start + quantity - 1 > 65535)
        return;

    m_editTransactionId.GetWindowText(text);
    if (!ParseUnsignedDecimal(text, 65535, transactionId))
        return;

    m_editProtocolId.GetWindowText(text);
    if (!ParseUnsignedDecimal(text, 0, protocolId))
        return;

    std::vector<BYTE> frame =
    {
        static_cast<BYTE>(transactionId >> 8),
        static_cast<BYTE>(transactionId & 0xFF),
        static_cast<BYTE>(protocolId >> 8),
        static_cast<BYTE>(protocolId & 0xFF),
        0x00, 0x06,
        static_cast<BYTE>(address),
        0x03,
        static_cast<BYTE>(start >> 8),
        static_cast<BYTE>(start & 0xFF),
        static_cast<BYTE>(quantity >> 8),
        static_cast<BYTE>(quantity & 0xFF)
    };

    CString output;

    for (size_t i = 0; i < frame.size(); ++i)
    {
        CString part;
        part.Format(_T("%02X"), frame[i]);

        if (i > 0)
            output += _T(" ");

        output += part;
    }

    m_editOutput.SetWindowText(output);
}

void CFrameBuilderPage::OnBnClickedCopyFrame()
{
    CString frame;
    m_editOutput.GetWindowText(frame);

    frame.Trim();

    if (frame.IsEmpty())
    {
        AfxMessageBox(_T("Generate a frame first."));
        return;
    }

    if (!OpenClipboard())
    {
        AfxMessageBox(_T("Unable to open clipboard."));
        return;
    }

    EmptyClipboard();

    const SIZE_T bytes =
        (frame.GetLength() + 1) * sizeof(TCHAR);

    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);

    if (memory == nullptr)
    {
        CloseClipboard();
        AfxMessageBox(_T("Unable to allocate clipboard memory."));
        return;
    }

    LPTSTR destination =
        static_cast<LPTSTR>(GlobalLock(memory));

    if (destination == nullptr)
    {
        GlobalFree(memory);
        CloseClipboard();
        AfxMessageBox(_T("Unable to access clipboard memory."));
        return;
    }

    memcpy(destination, frame.GetString(), bytes);
    GlobalUnlock(memory);

#ifdef _UNICODE
    if (SetClipboardData(CF_UNICODETEXT, memory) == nullptr)
#else
    if (SetClipboardData(CF_TEXT, memory) == nullptr)
#endif
    {
        GlobalFree(memory);
        CloseClipboard();
        AfxMessageBox(_T("Unable to copy frame."));
        return;
    }

    CloseClipboard();
}
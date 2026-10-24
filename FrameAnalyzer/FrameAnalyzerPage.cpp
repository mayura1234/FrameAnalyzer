
#include "pch.h"
#include "framework.h"
#include "FrameAnalyzerPage.h"
#include "afxdialogex.h"
#include <tchar.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    CString GetModbusFunctionName(unsigned char functionCode)
    {
        switch (functionCode)
        {
        case 0x01: return _T("Read Coils");
        case 0x02: return _T("Read Discrete Inputs");
        case 0x03: return _T("Read Holding Registers");
        case 0x04: return _T("Read Input Registers");
        case 0x05: return _T("Write Single Coil");
        case 0x06: return _T("Write Single Register");
        case 0x0F: return _T("Write Multiple Coils");
        case 0x10: return _T("Write Multiple Registers");
        default:   return _T("Unknown Function");
        }
    }

    CString FormatHexBytes(
        const std::vector<unsigned char>& bytes,
        int start,
        int count)
    {
        CString result;

        for (int i = 0; i < count; ++i)
        {
            CString oneByte;
            oneByte.Format(_T("%02X"), bytes[start + i]);

            if (!result.IsEmpty())
                result += _T(" ");

            result += oneByte;
        }

        return result;
    }
}

CFrameAnalyzerPage::CFrameAnalyzerPage(CWnd* pParent)
    : CDialogEx(IDD_FRAME_ANALYZER_PAGE, pParent)
{
}

void CFrameAnalyzerPage::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_EDIT_HEX_INPUT, m_strHexInput);
    DDX_Text(pDX, IDC_EDIT_OUTPUT, m_strOutput);
    DDX_Control(pDX, IDC_COMBO_PROTOCOL, m_cbProtocol);
}

BEGIN_MESSAGE_MAP(CFrameAnalyzerPage, CDialogEx)
    ON_BN_CLICKED(
        IDC_BTN_DECODE,
        &CFrameAnalyzerPage::OnBnClickedBtnDecode)
END_MESSAGE_MAP()

BOOL CFrameAnalyzerPage::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_cbProtocol.ResetContent();
    m_cbProtocol.AddString(_T("Modbus RTU"));

    // Add further supported protocols here in the future.
    // m_cbProtocol.AddString(_T("Modbus TCP"));
    // m_cbProtocol.AddString(_T("Modbus ASCII"));
    // m_cbProtocol.AddString(_T("IEC 60870-5-101"));
    // m_cbProtocol.AddString(_T("IEC 60870-5-104"));
    // m_cbProtocol.AddString(_T("DNP3"));

    m_cbProtocol.SetCurSel(0);

    return TRUE;
}

void CFrameAnalyzerPage::OnBnClickedBtnDecode()
{
    UpdateData(TRUE);

    if (m_strHexInput.IsEmpty())
    {
        m_strOutput =
            _T("Error: Please enter a hexadecimal frame.");

        UpdateData(FALSE);
        return;
    }

    std::vector<unsigned char> bytes;
    CString errorMessage;

    if (!TryParseHexInput(m_strHexInput, bytes, errorMessage))
    {
        m_strOutput = _T("Hex Input Error\r\n") + errorMessage;
        UpdateData(FALSE);
        return;
    }

    if (bytes.size() < 4)
    {
        m_strOutput =
            _T("Error: Modbus RTU frame is too short.");

        UpdateData(FALSE);
        return;
    }

    const int frameLength = static_cast<int>(bytes.size());

    const unsigned char address = bytes[0];
    const unsigned char functionCode = bytes[1];

    CString functionName;

    // Exception responses have the highest bit set.
    if ((functionCode & 0x80) != 0)
    {
        const unsigned char originalFunction =
            functionCode & 0x7F;

        functionName.Format(
            _T("Exception Response - %s"),
            GetModbusFunctionName(originalFunction));
    }
    else
    {
        functionName = GetModbusFunctionName(functionCode);
    }

    // Bytes after the function code and before the CRC.
    const CString payload =
        FormatHexBytes(bytes, 2, frameLength - 4);

    // Modbus RTU transmits the CRC low byte first.
    const unsigned short receivedCrc =
        static_cast<unsigned short>(
            bytes[frameLength - 2] |
            (static_cast<unsigned short>(
                bytes[frameLength - 1]) << 8));

    const unsigned short calculatedCrc =
        CalculateCRC(bytes, frameLength - 2);

    const CString crcStatus =
        (receivedCrc == calculatedCrc)
        ? _T("PASS")
        : _T("FAIL");

    CString result;

    result.Format(
        _T("\r\n\r\n")
        _T("Address       : %u (0x%02X)\r\n")
        _T("Function Code : %02X - %s\r\n")
        _T("Frame Length  : %d bytes\r\n")
        _T("Payload       : %s\r\n\r\n")
        _T("CRC Received  : 0x%04X\r\n")
        _T("CRC Calculated: 0x%04X\r\n")
        _T("CRC Status    : %s"),
        static_cast<unsigned int>(address),
        address,
        functionCode,
        functionName,
        frameLength,
        payload.IsEmpty() ? _T("(empty)") : payload,
        receivedCrc,
        calculatedCrc,
        crcStatus);

    m_strOutput = result;
    UpdateData(FALSE);
}

bool CFrameAnalyzerPage::TryParseHexInput(
    const CString& input,
    std::vector<unsigned char>& frameBytes,
    CString& errorMessage)
{
    frameBytes.clear();
    errorMessage.Empty();

    CString cleaned;

    // Ignore whitespace, but reject other non-hex characters.
    for (int i = 0; i < input.GetLength(); ++i)
    {
        const TCHAR ch = input[i];

        if (_istspace(ch))
            continue;

        if (!_istxdigit(ch))
        {
            errorMessage.Format(
                _T("Invalid character '%c' at input position %d."),
                ch,
                i + 1);

            return false;
        }

        cleaned += ch;
    }

    if (cleaned.IsEmpty())
    {
        errorMessage = _T("No hexadecimal digits were provided.");
        return false;
    }

    if (cleaned.GetLength() % 2 != 0)
    {
        errorMessage =
            _T("Odd number of hexadecimal digits. Each byte needs 2 digits.");

        return false;
    }

    auto hexValue = [](TCHAR ch) -> int
        {
            if (ch >= _T('0') && ch <= _T('9'))
                return ch - _T('0');

            if (ch >= _T('A') && ch <= _T('F'))
                return ch - _T('A') + 10;

            if (ch >= _T('a') && ch <= _T('f'))
                return ch - _T('a') + 10;

            return -1;
        };

    for (int i = 0; i < cleaned.GetLength(); i += 2)
    {
        const int highValue = hexValue(cleaned[i]);
        const int lowValue = hexValue(cleaned[i + 1]);

        if (highValue < 0 || lowValue < 0)
        {
            errorMessage = _T("Invalid hexadecimal byte.");
            frameBytes.clear();
            return false;
        }

        frameBytes.push_back(
            static_cast<unsigned char>(
                (highValue << 4) | lowValue));
    }

    return true;
}

unsigned short CFrameAnalyzerPage::CalculateCRC(
    const std::vector<unsigned char>& data,
    int length)
{
    unsigned short crc = 0xFFFF;

    for (int pos = 0; pos < length; ++pos)
    {
        crc ^= static_cast<unsigned short>(data[pos]);

        for (int i = 8; i != 0; --i)
        {
            if ((crc & 0x0001) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}
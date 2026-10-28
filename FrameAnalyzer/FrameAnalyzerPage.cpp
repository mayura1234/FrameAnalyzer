
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
    CString GetFunctionDescription(unsigned char functionCode)
    {
        CString functionName;

        if ((functionCode & 0x80) != 0)
        {
            CString originalName =
                GetModbusFunctionName(functionCode & 0x7F);

            functionName.Format(
                _T("Exception Response - %s"),
                originalName);
        }
        else
        {
            functionName = GetModbusFunctionName(functionCode);
        }

        return functionName;
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

    unsigned int ReadUInt16BE(
        const std::vector<unsigned char>& bytes,
        int offset)
    {
        return
            (static_cast<unsigned int>(bytes[offset]) << 8) |
            static_cast<unsigned int>(bytes[offset + 1]);
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
    m_cbProtocol.AddString(_T("Modbus ASCII"));
    m_cbProtocol.AddString(_T("Modbus TCP"));
     
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

    const int selectedProtocol = m_cbProtocol.GetCurSel();

    if (selectedProtocol == CB_ERR)
    {
        m_strOutput =
            _T("Error: Please select a protocol.");

        UpdateData(FALSE);
        return;
    }

    CString result;
    CString errorMessage;
    std::vector<unsigned char> bytes;


    switch (selectedProtocol)
    {
    case 0: // Modbus RTU
    {
        if (!TryParseHexInput(m_strHexInput, bytes, errorMessage))
        {
            result = _T("Hex Input Error\r\n") + errorMessage;
            break;
        }

        DecodeModbusRTU(bytes, result);
        break;
    }

    case 1: // Modbus ASCII
    {
        DecodeModbusASCII(m_strHexInput, result);
        break;
    }

    case 2: // Modbus TCP
    {
        if (!TryParseHexInput(m_strHexInput, bytes, errorMessage))
        {
            result = _T("Hex Input Error\r\n") + errorMessage;
            break;
        }

        DecodeModbusTCP(bytes, result);
        break;
    }

    default:
        result = _T("Error: Unsupported protocol selection.");
        break;
    }

    m_strOutput = result;
    UpdateData(FALSE);
}

void CFrameAnalyzerPage::DecodeModbusRTU(
    const std::vector<unsigned char>& bytes,
    CString& result)
{
    if (bytes.size() < 4)
    {
        result =
            _T("Modbus RTU Error\r\n")
            _T("The frame is too short. At least 4 bytes are required.");

        return;
    }

    const int frameLength = static_cast<int>(bytes.size());

    const unsigned char address = bytes[0];
    const unsigned char functionCode = bytes[1];

    const CString functionName =
        GetFunctionDescription(functionCode);

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

    result.Format(
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
}


void CFrameAnalyzerPage::DecodeModbusASCII(
    const CString& input,
    CString& result)
{
    // 1. Copy the input without automatically removing CR/LF.
    CString frame = input;

    // Remove leading spaces and tabs.
    while (!frame.IsEmpty() &&
        (frame[0] == _T(' ') || frame[0] == _T('\t')))
    {
        frame.Delete(0, 1);
    }

    // Remove trailing spaces and tabs.
    while (!frame.IsEmpty())
    {
        int last = frame.GetLength() - 1;
        TCHAR ch = frame[last];

        if (ch == _T(' ') || ch == _T('\t'))
        {
            frame.Delete(last, 1);
        }
        else
        {
            break;
        }
    }

    // 2. CR/LF is optional for pasted offline frames.
    bool terminatorPresent = false;

    if (frame.GetLength() >= 2 &&
        frame.Right(2) == _T("\r\n"))
    {
        // Remove a valid CR/LF terminator.
        frame = frame.Left(frame.GetLength() - 2);
        terminatorPresent = true;
    }
    else if (frame.Find(_T('\r')) != -1 ||
        frame.Find(_T('\n')) != -1)
    {
        result =
            _T("Invalid Modbus ASCII frame.\r\n")
            _T("If a terminator is supplied, it must be CR/LF.");

        return;
    }

    // 3. Validate the starting colon.
    if (frame.IsEmpty() || frame[0] != _T(':'))
    {
        result =
            _T("Invalid Modbus ASCII frame.\r\n")
            _T("Missing start character ':'.");
        return;
    }

    // 4. Extract hexadecimal content after ':'.
    CString hexContent = frame.Mid(1);

    if (hexContent.IsEmpty())
    {
        result =
            _T("Invalid Modbus ASCII frame.\r\n")
            _T("No hexadecimal data was supplied.");
        return;
    }

    // Each byte requires exactly two hexadecimal characters.
    if ((hexContent.GetLength() % 2) != 0)
    {
        result =
            _T("Invalid Modbus ASCII frame.\r\n")
            _T("Hexadecimal content must contain complete byte pairs.");
        return;
    }

    // 5. Convert hexadecimal text into bytes.
    std::vector<unsigned char> bytes;
    CString parseError;

    if (!TryParseHexInput(hexContent, bytes, parseError))
    {
        result =
            _T("Invalid Modbus ASCII hexadecimal data.\r\n") +
            parseError;
        return;
    }

    // Minimum frame: address + function code + LRC.
    if (bytes.size() < 3)
    {
        result =
            _T("Invalid Modbus ASCII frame.\r\n")
            _T("At least address, function code, and LRC are required.");
        return;
    }

    // 6. Extract the frame fields.
    unsigned char address = bytes[0];
    unsigned char functionCode = bytes[1];

    int byteCount = static_cast<int>(bytes.size());

    // Exclude address, function code, and LRC from the payload.
    CString payload = FormatHexBytes(
        bytes, 2, byteCount - 3);

    // Last byte is the received LRC.
    unsigned char receivedLRC = bytes[byteCount - 1];

    // Calculate LRC over every byte except the received LRC.
    unsigned char calculatedLRC = CalculateLRC(
        bytes, byteCount - 1);

    bool lrcValid = (receivedLRC == calculatedLRC);

    // 7. Describe the optional terminator.
    CString terminatorStatus = terminatorPresent
        ? _T("CR/LF supplied and valid")
        : _T("Not supplied (optional for pasted input)");

    // 8. Build the decode report.
    result.Format(
        _T("Address: 0x%02X (%u)\r\n")
        _T("Function Code: 0x%02X\r\n")
        _T("Function: %s\r\n")
        _T("Payload: %s\r\n\r\n")
        _T("LRC Received: 0x%02X\r\n")
        _T("LRC Calculated: 0x%02X\r\n")
        _T("LRC Status: %s\r\n")
        _T("Frame Terminator: %s\r\n")
        _T("Overall Status: %s"),
        address,
        address,
        functionCode,
        GetFunctionDescription(functionCode),
        payload.GetString(),
        receivedLRC,
        calculatedLRC,
        lrcValid ? _T("PASS") : _T("FAIL"),
        terminatorStatus.GetString(),
        lrcValid ? _T("PASS") : _T("FAIL"));
}


void CFrameAnalyzerPage::DecodeModbusTCP(
    const std::vector<unsigned char>& bytes,
    CString& result)
{
    // MBAP header is 7 bytes, followed by at least a function code.
    if (bytes.size() < 8)
    {
        result =
            _T("Modbus TCP Error\r\n")
            _T("The frame is too short. A complete MBAP header and function code are required.");

        return;
    }

    const unsigned int transactionId = ReadUInt16BE(bytes, 0);
    const unsigned int protocolId = ReadUInt16BE(bytes, 2);
    const unsigned int declaredLength = ReadUInt16BE(bytes, 4);
    const unsigned char unitId = bytes[6];

    // Length counts the Unit ID and the PDU, not the whole frame.
    const unsigned int expectedFrameLength = 6 + declaredLength;
    const unsigned int actualFrameLength =
        static_cast<unsigned int>(bytes.size());

    if (protocolId != 0)
    {
        result.Format(
            _T("Modbus TCP Error\r\n")
            _T("Invalid Protocol ID: 0x%04X. Modbus TCP requires 0x0000."),
            protocolId);

        return;
    }

    if (declaredLength < 2)
    {
        result =
            _T("Modbus TCP Error\r\n")
            _T("The MBAP Length must include at least the Unit ID and function code.");

        return;
    }

    if (expectedFrameLength != actualFrameLength)
    {
        result.Format(
            _T("Modbus TCP Error\r\n")
            _T("MBAP length mismatch.\r\n")
            _T("Length field       : %u bytes after the first 6 bytes\r\n")
            _T("Expected frame size: %u bytes\r\n")
            _T("Actual frame size  : %u bytes"),
            declaredLength,
            expectedFrameLength,
            actualFrameLength);

        return;
    }

    const unsigned char functionCode = bytes[7];
    const CString functionName = GetFunctionDescription(functionCode);

    const CString payload =
        FormatHexBytes(
            bytes,
            8,
            static_cast<int>(bytes.size()) - 8);

    result.Format(
        _T("Transaction ID : %u (0x%04X)\r\n")
        _T("Protocol ID    : %u (0x%04X)\r\n")
        _T("MBAP Length    : %u bytes\r\n")
        _T("Unit ID        : %u (0x%02X)\r\n")
        _T("Function Code  : %02X - %s\r\n")
        _T("Frame Length   : %u bytes\r\n")
        _T("PDU Payload    : %s\r\n\r\n")
        _T("MBAP Status    : PASS\r\n")
        _T("Checksum       : Not present in Modbus TCP"),
        transactionId,
        transactionId,
        protocolId,
        protocolId,
        declaredLength,
        static_cast<unsigned int>(unitId),
        unitId,
        functionCode,
        functionName,
        actualFrameLength,
        payload.IsEmpty() ? _T("(empty)") : payload);
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

unsigned char CFrameAnalyzerPage::CalculateLRC(
    const std::vector<unsigned char>& data,
    int length)
{
    unsigned int sum = 0;

    for (int i = 0; i < length; ++i)
        sum += data[i];

    // LRC is the two's complement of the low 8 bits of the sum.
    return static_cast<unsigned char>(
        (0x100 - (sum & 0xFF)) & 0xFF);
}
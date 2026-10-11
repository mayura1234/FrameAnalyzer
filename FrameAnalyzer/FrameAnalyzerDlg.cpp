	
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

	// --- PROTOCOL INITIALIZATION ---
	m_cbProtocol.ResetContent();
	m_cbProtocol.AddString(_T("Modbus RTU"));

	// Phase 2 / Other protocols
	/*
	m_cbProtocol.AddString(_T("IEC 60870-5-104"));
	m_cbProtocol.AddString(_T("IEC 60870-5-101"));
	m_cbProtocol.AddString(_T("DNP3"));

	*/

	// Select the first item in our programmatic list
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

//Decode Button 
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

	// Convert the text into our byte array
	std::vector<unsigned char> parsedBytes = ParseHexInput(m_strHexInput);

	// 1. Safety check: A Modbus frame must have at least 4 bytes (Address, Function, CRC, CRC)
	if (parsedBytes.size() < 4)
	{
		m_strOutput = _T("Error: Frame is too short to be valid.");
		UpdateData(FALSE);
		return;
	}

	int totalBytes = parsedBytes.size();

	// 1. Extract the Header
	unsigned char deviceAddress = parsedBytes[0];
	unsigned char functionCode = parsedBytes[1];

	CString functionName = _T("Unknown Function");
	bool isException = false;

	// Check if the highest bit (0x80) is set
	if (functionCode >= 0x80)
	{
		isException = true;
		// Remove the error flag (subtract 0x80) to find out which function originally failed
		int originalFunction = functionCode - 0x80;
		functionName.Format(_T("EXCEPTION on Function %d"), originalFunction);
	}
	else
	{
		if (functionCode == 3) functionName = _T("Read Holding Registers");
		else if (functionCode == 6) functionName = _T("Write Single Register");
	}

	// 2. Extract the CRC that was sent in the frame (the last two bytes)
	// Modbus sends the Low Byte first, then the High Byte
	unsigned char receivedCrcLow = parsedBytes[totalBytes - 2];
	unsigned char receivedCrcHigh = parsedBytes[totalBytes - 1];

	// Combine them into a single 16-bit number using bitwise shift (<<) and OR (|)
	unsigned short receivedCrc = (receivedCrcHigh << 8) | receivedCrcLow;

	// 3. Calculate what the CRC *should* be (exclude the last 2 bytes from the calculation)
	unsigned short calculatedCrc = CalculateCRC(parsedBytes, totalBytes - 2);

	// 4. Compare them for PASS/FAIL
	CString crcStatus;
	if (calculatedCrc == receivedCrc) {
		crcStatus = _T("PASS");
	}
	else {
		// %04X formats the integer as a 4-character uppercase Hex string
		crcStatus.Format(_T("FAIL (Expected %04X, got %04X)"), calculatedCrc, receivedCrc);
	}

	CString payloadInfo = _T("");

	if (isException && totalBytes >= 5)
	{
		// Byte 2 contains the specific error code
		unsigned char exceptionCode = parsedBytes[2];
		CString exceptionReason = _T("Unknown Error");

		if (exceptionCode == 1) exceptionReason = _T("Illegal Function (Device doesn't support this)");
		if (exceptionCode == 2) exceptionReason = _T("Illegal Data Address (Register doesn't exist)");
		if (exceptionCode == 3) exceptionReason = _T("Illegal Data Value (Bad data format)");
		if (exceptionCode == 4) exceptionReason = _T("Slave Device Failure (Device is broken)");

		payloadInfo.Format(_T("Error Code:      %d - %s\r\n"), exceptionCode, exceptionReason);
	}
	else if (functionCode == 3 && totalBytes == 8)
	{
		// Extract Starting Register (Byte 2 is High, Byte 3 is Low)
		unsigned short startAddress = (parsedBytes[2] << 8) | parsedBytes[3];
		// Extract Quantity to Read (Byte 4 is High, Byte 5 is Low)
		unsigned short quantity = (parsedBytes[4] << 8) | parsedBytes[5];

		payloadInfo.Format(_T("Start Register:  %d\r\nQuantity:        %d\r\n"), startAddress, quantity);
	}
	else if (functionCode == 6 && totalBytes == 8)
	{
		// Extract Target Register (Byte 2 is High, Byte 3 is Low)
		unsigned short registerAddress = (parsedBytes[2] << 8) | parsedBytes[3];

		// Extract Value to Write (Byte 4 is High, Byte 5 is Low)
		unsigned short writeValue = (parsedBytes[4] << 8) | parsedBytes[5];

		// We print the value in both decimal and HEX, because engineers 
		// often use specific hex codes (like 0xFF00) to trigger breakers
		payloadInfo.Format(_T("Target Register: %d\r\nWrite Value:     %d (0x%04X)\r\n"),
			registerAddress, writeValue, writeValue);
	}
	else
	{
		payloadInfo = _T("Payload:         (Hex Data Omitted)\r\n");
	}

	CString result;
	result.Format(_T("Device Address:  %d\r\nFunction Code:   %d  (%s)\r\n%sCRC Check:       %s"),
		deviceAddress,
		functionCode,
		functionName,
		payloadInfo,
		crcStatus);

	m_strOutput = result;
	UpdateData(FALSE);

}

// Parse the input text
std::vector<unsigned char> CFrameAnalyzerDlg::ParseHexInput(CString input)
{
	std::vector<unsigned char> frameBytes;

	// 1. Sanitize the input: strip out spaces and newlines
	input.Replace(_T(" "), _T(""));
	input.Replace(_T("\r"), _T(""));
	input.Replace(_T("\n"), _T(""));

	// 2. Loop through the string two characters at a time
	for (int i = 0; i < input.GetLength(); i += 2)
	{
		// Extract a 2-character chunk
		CString hexChunk = input.Mid(i, 2);

		// Convert those two hex characters into a real integer (Base 16)
		long byteValue = _tcstol(hexChunk, nullptr, 16);

		// Cast to an unsigned char (byte) and add it to our array
		frameBytes.push_back(static_cast<unsigned char>(byteValue));
	}

	return frameBytes;
}
unsigned short CFrameAnalyzerDlg::CalculateCRC(const std::vector<unsigned char>& data, int length)
{
	unsigned short crc = 0xFFFF;

	for (int pos = 0; pos < length; pos++)
	{
		crc ^= (unsigned short)data[pos];

		for (int i = 8; i != 0; i--)
		{
			if ((crc & 0x0001) != 0) {
				crc >>= 1;
				crc ^= 0xA001;
			}
			else {
				crc >>= 1;
			}
		}
	}
	return crc;
}
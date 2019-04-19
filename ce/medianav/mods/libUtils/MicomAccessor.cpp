#include "stdafx.h"
#include "utils.h"
#include "MicomAccessor.h"

#ifdef TESTMODE
#define DBG 1
#else
#define DBG 0
#endif

MicomAccessor::MicomAccessor()
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hReadThread = NULL;
	m_busy = FALSE;
	m_nBufferSize = 256;
	m_isLogOn = FALSE;
}

MicomAccessor::~MicomAccessor()
{
	this->Disconnect();
}

DWORD MicomAccessor::SendCommandEx(BYTE mgr, BYTE cmd, BYTE len, BYTE* buffer)
{
	if (m_busy) 
    {
        debugPrintf(DBG, L"MicomAccessor::SendCommandEx(): Micom is busy!\r\n");
		return 0;
	}

	if (!IsConnected()) 
    {
        debugPrintf(DBG, L"MicomAccessor::SendCommandEx(): Micom is disconnected!\r\n");
		return 0;
	}

	m_busy = TRUE;
	BYTE* mes = new BYTE[len + 5];
	mes[0] = 0xAA;
	mes[1] = (mgr << 4) ^ 1;
	mes[2] = cmd;
	mes[3] = len;
	if (len > 0)
		memcpy(&mes[4], buffer, len);
	mes[len + 4] = GetCheckSum(mes, len + 4);

//	if (m_isLogOn)
//		logger.writeCommand(mes, mgr, 1, cmd, len + 5);

	this->Send(mes, len + 5);
	delete[] mes;

	m_hReadThread = CreateThread(NULL, 0, ReadPortThread, this, 0, NULL);

	DWORD res = WaitForSingleObject(m_hReadThread, 500);
	CloseHandle(m_hReadThread);
	m_hReadThread = NULL;
	m_busy = FALSE;

	return res;
}

DWORD MicomAccessor::SendReadCmd(BYTE mgr, DWORD addr, BYTE len)
{
	if (m_busy) 
    {
        debugPrintf(DBG, L"MicomAccessor::SendReadCmd(): Micom is busy!\r\n");
		return 0;
	}

	if (!IsConnected()) 
    {
        debugPrintf(DBG, L"MicomAccessor::SendReadCmd(): Micom is disconnected!\r\n");
		return 0;
	}

	m_busy = TRUE;
	BYTE* mes = new BYTE[9];
	mes[0] = 0xAA;
	mes[1] = (mgr << 4) ^ 5;
	mes[2] = len;
	mes[3] = 4;
	*(DWORD*)&mes[4] = addr;
	mes[8] = GetCheckSum(mes, 8);

//	if (m_isLogOn)
//		logger.writeRead(mes, mgr, addr, 9);

	this->Send(mes, 9);
	delete[] mes;

	m_hReadThread = CreateThread(NULL, 0, ReadPortThread, this, 0, NULL);

	DWORD res = WaitForSingleObject(m_hReadThread, 500);
	CloseHandle(m_hReadThread);
	m_hReadThread = NULL;
	m_busy = FALSE;

	return res;
}

DWORD MicomAccessor::SendWriteCmd(BYTE mgr, DWORD addr, BYTE len, BYTE* buffer)
{
    if (m_busy) 
    {
        debugPrintf(DBG, L"MicomAccessor::SendWriteCmd(): Micom is busy!\r\n");
        return 0;
    }

    if (!IsConnected()) 
    {
        debugPrintf(DBG, L"MicomAccessor::SendWriteCmd(): Micom is disconnected!\r\n");
        return 0;
    }

	m_busy = TRUE;

	BYTE* mes = new BYTE[len + 9];
	mes[0] = 0xAA;
	mes[1] = (mgr << 4) ^ 6;
	mes[2] = len;
	mes[3] = len + 4;
	*(DWORD*)&mes[4] = addr;
	if (len > 0)
		memcpy(&mes[8], buffer, len);
	mes[len + 8] = GetCheckSum(mes, len + 8);

//	if (m_isLogOn)
//		logger.writeWrite(mes, mgr, addr, len + 9);

	this->Send(mes, len + 9);
	delete[] mes;

	m_hReadThread = CreateThread(NULL, 0, ReadPortThread, this, 0, NULL);

	DWORD res = WaitForSingleObject(m_hReadThread, 500);
	CloseHandle(m_hReadThread);
	m_hReadThread = NULL;
	m_busy = FALSE;

	return res;
}

ResRead* MicomAccessor::GetResRead()
{
	return &this->resRead;
}

DWORD WINAPI MicomAccessor::ReadPortThread(LPVOID lpParameter)
{
	MicomAccessor* m_ccmd;
	m_ccmd = (MicomAccessor*)lpParameter;

	BOOL fReadState;
	DWORD dwLength;

	fReadState = ReadFile(m_ccmd->m_hComm, m_ccmd->resRead.rData, m_ccmd->m_nBufferSize, &dwLength, NULL);
	m_ccmd->resRead.rLen = dwLength;

	while (dwLength != 0 && m_ccmd->resRead.rLen < m_ccmd->m_nBufferSize) {
		fReadState = ReadFile(m_ccmd->m_hComm, &m_ccmd->resRead.rData[m_ccmd->resRead.rLen], m_ccmd->m_nBufferSize - m_ccmd->resRead.rLen, &dwLength, NULL);
		m_ccmd->resRead.rLen += dwLength;
	}

//	if (m_ccmd->m_isLogOn)
//		logger.writeRThread(m_ccmd->resRead.rData, m_ccmd->resRead.rLen);

	if (m_ccmd->resRead.rLen > 4) {
		m_ccmd->resRead.mgr = (((m_ccmd->resRead.rData[1] & 0xf0) >> 4) & 0xf);
		m_ccmd->resRead.grp = (m_ccmd->resRead.rData[1] & 0xf);
		m_ccmd->resRead.cmd = m_ccmd->resRead.rData[2];
		m_ccmd->resRead.len = m_ccmd->resRead.rData[3];
		m_ccmd->resRead.data = &m_ccmd->resRead.rData[4];
		m_ccmd->resRead.checkSum = m_ccmd->resRead.rData[m_ccmd->resRead.rLen - 1];

		if (m_ccmd->resRead.checkSum != GetCheckSum(m_ccmd->resRead.rData, m_ccmd->resRead.rLen - 1)) {
			if (m_ccmd->resRead.checkSum == 0xA6) {
				m_ccmd->resRead.checkSum = m_ccmd->resRead.rData[m_ccmd->resRead.rLen - 2];
				if (m_ccmd->resRead.checkSum != GetCheckSum(m_ccmd->resRead.rData, m_ccmd->resRead.rLen - 2))
					m_ccmd->resRead.rLen = 0;
			} else {
				m_ccmd->resRead.rLen = 0;
			}
		}
	}

	return 0;
}

BOOL MicomAccessor::Connect()
{
	if (m_hComm == INVALID_HANDLE_VALUE)
		m_hComm = CreateFile(_T("COM2:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (m_hComm == INVALID_HANDLE_VALUE) 
    {
        debugPrintf(DBG, L"MicomAccessor::Connect(): Unable to open Micom port!\r\n");
		return FALSE;
	}

	DCB dcb;
	dcb.DCBlength = sizeof(DCB);
    GetCommState(m_hComm, &dcb);

    dcb.BaudRate = 300000;				// Current baud 
	dcb.ByteSize = 8;				    // Number of bits/byte, 4-8 
	dcb.fAbortOnError = FALSE;          // Do not abort reads/writes on error
	dcb.fBinary = TRUE;                 // Binary mode; no EOF check 
	dcb.fDsrSensitivity = FALSE;        // DSR sensitivity 
	dcb.fDtrControl = DTR_CONTROL_ENABLE;	// DTR flow control type 
	dcb.fErrorChar = FALSE;             // Disable error replacement 
	dcb.fInX = FALSE;                   // No XON/XOFF in flow control 
	dcb.fNull = FALSE;                  // Disable null stripping 
	dcb.fOutX = FALSE;                  // No XON/XOFF out flow control 
	dcb.fOutxCtsFlow = FALSE;           // No CTS output flow control 
	dcb.fOutxDsrFlow = FALSE;           // No DSR output flow control 
	dcb.fParity = FALSE;                // Enable parity checking 
	dcb.fRtsControl = RTS_CONTROL_ENABLE;	// RTS flow control 
	dcb.fTXContinueOnXoff = FALSE;      // XOFF continues Tx 
	dcb.Parity = NOPARITY;              // 0-4=no,odd,even,mark,space
	dcb.StopBits = ONESTOPBIT;          // 0,1,2 = 1, 1.5, 2 
	   
	if (!SetCommState(m_hComm, &dcb)) 
    {
		CloseHandle(m_hComm);   
        debugPrintf(DBG, L"MicomAccessor::Connect(): SetCommState() failed!\r\n");
		return FALSE ;
	}

	COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout = 2;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 1;
    CommTimeOuts.ReadTotalTimeoutConstant = 6;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant = 0;

	if (!SetCommTimeouts(m_hComm, &CommTimeOuts)) 
    {
		CloseHandle(m_hComm);   
        debugPrintf(DBG, L"MicomAccessor::Connect(): SetCommTimeouts() failed!\r\n");
		return FALSE ;
	}

	return TRUE;
}

BOOL MicomAccessor::Disconnect()
{
	if (m_hComm != INVALID_HANDLE_VALUE) {
		SetCommMask(m_hComm, 0);
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}

	if (m_busy) {
		m_busy = FALSE;
		WaitForSingleObject(m_hReadThread, 500);
		CloseHandle(m_hReadThread);
		m_hReadThread = NULL;
	}

	return TRUE;
}

BOOL MicomAccessor::IsConnected()
{   
    return (m_hComm != NULL);   
}

BYTE MicomAccessor::GetCheckSum(BYTE* buffer, BYTE len)
{
	BYTE result = 0;

	for (BYTE i=0;i<len;i++)
		result ^= buffer[i];

	return result;
}

DWORD MicomAccessor::Send(BYTE* mes, BYTE len)
{
	DWORD dwBytesWritten = 0;
	
	if (!WriteFile(m_hComm, mes, len, &dwBytesWritten, NULL)) 
    {
        debugPrintf(DBG, L"MicomAccessor::Send(): WriteFile() failed!\r\n");
		return 0;
	}

	return dwBytesWritten;
}

BOOL MicomAccessor::ToggleLogger()
{
	m_isLogOn = !m_isLogOn;
	return m_isLogOn;
}

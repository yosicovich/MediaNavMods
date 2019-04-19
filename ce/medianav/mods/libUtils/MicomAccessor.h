
#pragma once

#include "micom.h"

struct ResRead {
	BYTE mgr;
	BYTE grp;
	BYTE cmd;
	BYTE len;
	BYTE* data;
	BYTE checkSum;

	BYTE rData[256];
	int rLen;
};

class MicomAccessor
{

public:
	MicomAccessor();
	~MicomAccessor();

	DWORD SendCommandEx(BYTE mgr, BYTE cmd, BYTE len, BYTE* buffer);
	DWORD SendReadCmd(BYTE mgr, DWORD addr, BYTE len);
	DWORD SendWriteCmd(BYTE mgr, DWORD addr, BYTE len, BYTE* buffer);

	BOOL Connect();
	BOOL Disconnect();
	BOOL IsConnected();

	BOOL ToggleLogger();

	ResRead* GetResRead();

	static DWORD WINAPI ReadPortThread(LPVOID lpParameter);
	static BYTE GetCheckSum(BYTE* buffer, BYTE len);

private:
	DWORD Send(BYTE* mes, BYTE len);

	HANDLE m_hComm;
	HANDLE m_hReadThread;
	BOOL m_busy;
	BOOL m_isLogOn;
	int m_nBufferSize;

	ResRead resRead;

};
#pragma once

#include "Common.h"

class ITcpPacketReceiveCallback
{
public:
	virtual void TcpPacketReceive(const char *pData, int len) = 0;
};

class CTCPServer
{
private:
	HANDLE m_hThread;
	bool m_IsRunning;
	unsigned int static WINAPI ProcessThread(void *p);
	void Run();
	int m_Port;
	ITcpPacketReceiveCallback *m_pReceiveListener;
	char *m_DataToSend;
	int m_DataSize;
	int m_DataRemain;
	bool m_IsConnected;
	DWORD m_LastDataReceive;
public:
	CTCPServer(int port, ITcpPacketReceiveCallback *pReceiveListener);
	~CTCPServer();
	void SendBuffer(const char *data, int len);
	bool IsReady();
	bool IsConnected();
};


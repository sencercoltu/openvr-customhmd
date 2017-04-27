#pragma once

#ifndef TcpServer_h
#define TcpServer_h

#include "Common.h"

typedef void (*pfnTcpPacketReceiveCallback)(void *dst, const char *pData, int len);

class CTCPServer
{
private:
	HANDLE m_hThread;
	bool m_IsRunning;
	unsigned int static WINAPI ProcessThread(void *p);
	void Run();
	int m_Port;
	pfnTcpPacketReceiveCallback pfPacketCallback;
	char *m_DataToSend;
	int m_DataSize;
	int m_DataRemain;
	bool m_IsConnected;
	DWORD m_LastDataReceive;
	void *m_Dest;
public:
	CTCPServer(int port, pfnTcpPacketReceiveCallback cb, void *dst);
	~CTCPServer();
	void SendBuffer(const char *data, int len);
	bool IsReady();
	bool IsConnected();
};

#endif //TcpServer_h
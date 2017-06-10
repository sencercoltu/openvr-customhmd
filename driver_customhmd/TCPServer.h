#pragma once

#ifndef TcpServer_h
#define TcpServer_h

#include "Common.h"

enum VirtualPacketTypes
{
	Invalid = 0,
	VrFrameInit = 1,
	VrFrame = 2,
	Rotation = 3,
	CameraFrameInit = 4,
	CameraFrame = 5,
	CameraAction = 6
};

typedef void (*pfnTcpPacketReceiveCallback)(void *dst, VirtualPacketTypes type, const char *pData, int len);

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
	uint16_t m_Sequence;
	VirtualPacketTypes m_DataType;
	int m_DataRemain;
	bool m_IsConnected;
	DWORD m_LastDataReceive;
	void *m_Dest;
	bool *m_pLastCameraStatus;
public:
	CTCPServer(int port, pfnTcpPacketReceiveCallback cb, void *dst);
	~CTCPServer();
	void SendBuffer(VirtualPacketTypes type, const char *data, int len);
	bool IsReady();
	bool IsConnected();
	void SetCameraStatus(bool *pEn) { m_pLastCameraStatus = pEn; }
};

#endif //TcpServer_h
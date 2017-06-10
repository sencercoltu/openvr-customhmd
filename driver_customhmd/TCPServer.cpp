#include "TCPServer.h"

CTCPServer::CTCPServer(int port, pfnTcpPacketReceiveCallback cb, void *dst)
{
	m_pLastCameraStatus = nullptr;
	m_Dest = dst;
	m_LastDataReceive = 0;
	m_IsConnected = false;
	m_DataSize = m_DataRemain = 0;
	m_DataToSend = nullptr;

	pfPacketCallback = cb;
	m_Port = port;	
	m_hThread = nullptr;
	m_IsRunning = false;
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ProcessThread, this, CREATE_SUSPENDED, nullptr));
	if (m_hThread)
	{
		m_IsRunning = true;
		ResumeThread(m_hThread);
	}
}

CTCPServer::~CTCPServer()
{
	m_IsRunning = false;
	SAFE_THREADCLOSE(m_hThread);
}

unsigned int WINAPI CTCPServer::ProcessThread(void *p)
{
	auto pTcpServer = static_cast<CTCPServer *>(p);
	if (pTcpServer)
		pTcpServer->Run();
	_endthreadex(0);
	return 0;
}


//#define DEFAULT_BUFLEN 32
#define MAX_SEND_SIZE 32768

void CTCPServer::Run()
{
	fd_set rd, wr;
	timeval timeout;

	SOCKET m_ServerSocket = INVALID_SOCKET;
	SOCKET m_ClientSocket = INVALID_SOCKET;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iResult;
	char *recvBuf = (char *)malloc(1024 * 1024 * 10);
	int bufPos = 0;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	struct tPacketHeader
	{
		uint32_t Length;
		uint16_t Type;
		uint16_t Sequence;
	};

	tPacketHeader *pHeader = (tPacketHeader *)recvBuf;

	char szPort[64] = {};
	sprintf_s(szPort, "%u", m_Port);
	iResult = getaddrinfo(NULL, szPort, &hints, &result);

	while (m_IsRunning)
	{
		if (pfPacketCallback)
			pfPacketCallback(m_Dest, VirtualPacketTypes::Invalid, nullptr, 0);
		m_IsConnected = false;
		m_ServerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (m_ServerSocket == INVALID_SOCKET)
			break;
		int opt = 1;
		if (setsockopt(m_ServerSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(int)) < 0)
			break;

		unsigned long mode = 1;
		if (ioctlsocket(m_ServerSocket, FIONBIO, &mode) != 0)
			break;

		iResult = bind(m_ServerSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
			break;

		iResult = listen(m_ServerSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
			break;


		while (m_IsRunning)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			FD_ZERO(&rd);
			FD_ZERO(&wr);
			FD_SET(m_ClientSocket, &wr);
			FD_SET(m_ServerSocket , &rd);
			int res = select((int)m_ServerSocket + 1, &rd, nullptr, nullptr, &timeout);
			if (res == SOCKET_ERROR)
			{
				m_IsRunning = false;
				break;
			}
			else if (res > 0)
			{
				if (FD_ISSET(m_ServerSocket, &rd))
				{
					m_ClientSocket = accept(m_ServerSocket, NULL, NULL);
					closesocket(m_ServerSocket);
					m_ServerSocket = INVALID_SOCKET;

					unsigned long mode = 1;
					ioctlsocket(m_ClientSocket, FIONBIO, &mode);
					
					m_DataSize = m_DataRemain = 0;
					m_DataToSend = nullptr;
					m_IsConnected = true;
					m_LastDataReceive = GetTickCount();
					bufPos = 0;
					break;
				}
			}
			Sleep(100);
		}

		
		while (m_IsRunning)
		{
			if (GetTickCount() - m_LastDataReceive > 2000)
				break;

			timeout.tv_sec = 0;
			timeout.tv_usec = 1000;
			FD_ZERO(&rd);
			FD_ZERO(&wr);
			FD_SET(m_ClientSocket, &rd);
			FD_SET(m_ClientSocket, &wr);
			
			int res = select((int)m_ClientSocket + 1, &rd, &wr, nullptr, &timeout);
			if (res == SOCKET_ERROR)
				break;
			else if (res > 0)
			{
				if (FD_ISSET(m_ClientSocket, &rd))
				{
					//receive first 8 bytes 
					if (bufPos < sizeof(tPacketHeader))
					{
						res = recv(m_ClientSocket, &recvBuf[bufPos], 8 - bufPos, 0);
						bufPos += 8;
					}
					else
					{
						//we have the header, so we know the remaining length
						int totalLength = (8 + pHeader->Length);
						res = recv(m_ClientSocket, &recvBuf[bufPos], totalLength - bufPos, 0);
						if (res > 0) //callback 
						{
							bufPos += res;
							if (bufPos == totalLength)
							{
								if (pfPacketCallback)
								{
#ifdef _DEBUG
									wchar_t szDbg[128];
									wsprintf(szDbg, L"Out: %u, In: %u, Diff: %d\n", m_Sequence, pHeader->Sequence, m_Sequence - pHeader->Sequence);
									OutputDebugString(szDbg);
#endif //_DEBUG
									pfPacketCallback(m_Dest, (VirtualPacketTypes)pHeader->Type, &recvBuf[sizeof(tPacketHeader)], pHeader->Length);
								}
								m_LastDataReceive = GetTickCount();
								bufPos = 0;
							}
							else if (bufPos > totalLength)
								break;
						}
						else if (res == 0)
							break;
						else if (WSAGetLastError() != WSAEWOULDBLOCK)
							break;
					}
						
				}
				
				if (FD_ISSET(m_ClientSocket, &wr))
				{
					int res;
					if (m_DataRemain)
					{
						if (m_DataRemain == m_DataSize)
						{
							res = send(m_ClientSocket, (const char *)&m_DataSize, sizeof(int), 0);
							res = send(m_ClientSocket, (const char *)&m_DataType, sizeof(uint16_t), 0);
							res = send(m_ClientSocket, (const char *)&m_Sequence, sizeof(uint16_t), 0);
							m_Sequence++;
						}
						int sz = min(m_DataRemain, MAX_SEND_SIZE);
						res = send(m_ClientSocket, m_DataToSend, sz, 0);
						if (res == SOCKET_ERROR)
							break;
						if (res > 0)
						{
							m_DataToSend += res;
							m_DataRemain -= res;
							if (m_DataRemain == 0)
							{
								m_DataSize = 0;
								m_DataToSend = nullptr;
								//OutputDebugString(L"Send done\n");
							}
						}
					}
					else if (m_pLastCameraStatus)
					{
						int size = sizeof(int);
						int status = (*m_pLastCameraStatus) ? 1 : 0;
						m_pLastCameraStatus = nullptr;
						int type = VirtualPacketTypes::CameraAction;
						res = send(m_ClientSocket, (const char *)&size, sizeof(int), 0);
						res = send(m_ClientSocket, (const char *)&type, sizeof(uint16_t), 0);
						res = send(m_ClientSocket, (const char *)&m_Sequence, sizeof(uint16_t), 0);
						m_Sequence++;
						res = send(m_ClientSocket, (const char *)&status, sizeof(int), 0);
					}
				}
			}
		}

		if (m_ClientSocket != INVALID_SOCKET)
			closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
		m_IsConnected = false;
		Sleep(100);
	}


	m_IsConnected = false;

	if (m_ServerSocket != INVALID_SOCKET)
		closesocket(m_ServerSocket);
	m_ServerSocket = INVALID_SOCKET;

	if (m_ClientSocket != INVALID_SOCKET)
		closesocket(m_ClientSocket);
	m_ClientSocket = INVALID_SOCKET;

	freeaddrinfo(result);

	if (recvBuf)
		free(recvBuf);
	recvBuf = nullptr;

	WSACleanup();
}

bool CTCPServer::IsReady()
{
	return (m_DataSize == 0) && m_IsConnected;
}

bool CTCPServer::IsConnected()
{
	return m_IsConnected;
}

void CTCPServer::SendBuffer(VirtualPacketTypes type, const char *pData, int len)
{
	if (!m_IsConnected) return;
	m_DataType = type;
	m_DataRemain = len;
	m_DataSize = len;
	m_DataToSend = (char *) pData;
}


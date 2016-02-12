/** Serial.cpp
 *
 * A very simple serial port control class that does NOT require MFC/AFX.
 *
 * @author Hans de Ruiter
 *
 * @version 0.1 -- 28 October 2008
 */

#include <iostream>
using namespace std;

#include "Serial.h"

CSerial::CSerial(std::wstring &commPortName, int bitRate)
{
	m_hCommHandle = CreateFile(commPortName.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if(m_hCommHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	else 
	{
		COMMTIMEOUTS cto = { MAXDWORD, 0, 0, 0, 0};
		DCB dcb;
		if(!SetCommTimeouts(m_hCommHandle,&cto))
		{
			CSerial::~CSerial();
		}

		// set DCB
		memset(&dcb,0,sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = bitRate;
		dcb.fBinary = 1;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.ByteSize = 8;

		if(!SetCommState(m_hCommHandle,&dcb))
		{
			CSerial::~CSerial();			
		}
	}
}

CSerial::~CSerial()
{
	CloseHandle(m_hCommHandle);
	m_hCommHandle = INVALID_HANDLE_VALUE;
}

int CSerial::Write(const char *buffer, int buffLen)
{
	DWORD numWritten;
	WriteFile(m_hCommHandle, buffer, buffLen, &numWritten, NULL);
	return numWritten;
}

int CSerial::Read(char *buffer, int buffLen)
{
	DWORD numRead;
	BOOL ret = ReadFile(m_hCommHandle, buffer, buffLen, &numRead, NULL);
	if(!ret)	
		return 0;	
	return numRead;
}

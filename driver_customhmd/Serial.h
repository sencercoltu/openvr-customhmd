#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <string>
#include <windows.h>

class CSerial
{
private:
	HANDLE m_hCommHandle;
public:
	CSerial(std::wstring &commPortName, int bitRate = 115200);
	virtual ~CSerial();	
	int Write(const char *buffer, int buffLen);
	int Read(char *buffer, int buffLen);
};

#endif //__SERIAL_H__

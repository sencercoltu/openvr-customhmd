#include "ShMem.h"
#include <aclapi.h>

CShMem::CShMem()
{
	WatchDogEnabled = false;
	_accessLock = CreateMutex(nullptr, false, L"Global\\CustomHMDCommLock");
	_accessor = nullptr;
	_sharedMem = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, _bufferSize, L"CustomHMDComm");
	if (_sharedMem) 	
	{
		SetSecurityInfo(_sharedMem, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, (PACL)nullptr, nullptr);
		_accessor = (char *)MapViewOfFile(_sharedMem, FILE_MAP_ALL_ACCESS, 0, 0, _bufferSize);
	}
}


CShMem::~CShMem()
{
	if (_accessor) UnmapViewOfFile(_accessor);
	_accessor = nullptr;
	if (_sharedMem) CloseHandle(_sharedMem);
	_sharedMem = nullptr;
	CloseHandle(_accessLock);
	_accessLock = nullptr;
}

CommState CShMem::GetState()
{
	if (WaitForSingleObject(_accessLock, 100) == WAIT_OBJECT_0)
	{
		_status = *((CommStatus*)&_accessor[_statusOffset]);
		WatchDogEnabled = _status.EnableWatchDog ? true : false;
		ReleaseMutex(_accessLock);
	}	
	return _status.State;		
}

void CShMem::WriteOutgoingPacket(char *packet)
{
	if (WaitForSingleObject(_accessLock, 100) == WAIT_OBJECT_0)
	{
		_status = *((CommStatus*)&_accessor[_statusOffset]);
		_status.DriverTime = GetTickCount();
		if (_status.OutgoingPackets < _maxPackets)
		{
			int offset = _outgoingOffset + (_status.OutgoingPackets * _packetSize);
			for (int i = 0; i < _packetSize; i++)
				_accessor[offset + i] = packet[i];
			_status.OutgoingPackets++;
			*((CommStatus*)&_accessor[_statusOffset]) = _status;
		}
		else
		{
			//reset buffer
			OutputDebugString(L"Buffer full\n");
			_status.OutgoingPackets = 0;
			*((CommStatus*)&_accessor[_statusOffset]) = _status;
		}
		ReleaseMutex(_accessLock);
	}
}

char *CShMem::ReadIncomingPackets(int *count)
{
	*count = 0;
	char *result = nullptr;
	if (WaitForSingleObject(_accessLock, 100) == WAIT_OBJECT_0)
	{
		_status = *((CommStatus*)&_accessor[_statusOffset]);
		_status.DriverTime = GetTickCount();
		if (_status.IncomingPackets > 0)
		{
			result = new char[_status.IncomingPackets * _packetSize];
			int pos = 0;
			for (auto p = 0; p < _status.IncomingPackets; p++)
			{				
				auto currOffset = _incomingOffset + (p * _packetSize);
				for (auto i = 0; i < _packetSize; i++)
					result[pos++] = _accessor[currOffset + i];
			}
			*count = _status.IncomingPackets;
			_status.IncomingPackets = 0;			
		}
		*((CommStatus*)&_accessor[_statusOffset]) = _status;
		ReleaseMutex(_accessLock);
	}
	return result;
}

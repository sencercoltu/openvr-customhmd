#include "ShMem.h"
#include <aclapi.h>

CShMem::CShMem()
{
	WatchDogEnabled = false;
	_commAccessLock = CreateMutex(nullptr, false, L"Global\\CustomHMDCommLock");
	_commAccessor = nullptr;
	_commSharedMem = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, _commBufferSize, L"CustomHMDComm");
	if (_commSharedMem)
	{
		SetSecurityInfo(_commSharedMem, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, (PACL)nullptr, nullptr);
		_commAccessor = (char *)MapViewOfFile(_commSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, _commBufferSize);
	}

	_screenAccessLock[0] = CreateMutex(nullptr, false, L"Global\\CustomHMDLeftLock");
	_screenAccessor[0] = nullptr;
	_screenSharedMem[0] = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, _screenBufferSize, L"CustomHMDLeft");
	if (_screenSharedMem[0])
	{
		SetSecurityInfo(_screenSharedMem[0], SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, (PACL)nullptr, nullptr);
		_screenAccessor[0] = (char *)MapViewOfFile(_screenSharedMem[0], FILE_MAP_ALL_ACCESS, 0, 0, _screenBufferSize);
	}

	_screenAccessLock[1] = CreateMutex(nullptr, false, L"Global\\CustomHMDRightLock");
	_screenAccessor[1] = nullptr;
	_screenSharedMem[1] = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, _screenBufferSize, L"CustomHMDRight");
	if (_screenSharedMem[1])
	{
		SetSecurityInfo(_screenSharedMem[1], SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, (PACL)nullptr, nullptr);
		_screenAccessor[1] = (char *)MapViewOfFile(_screenSharedMem[1], FILE_MAP_ALL_ACCESS, 0, 0, _screenBufferSize);
	}
}


CShMem::~CShMem()
{
	if (_commAccessor) UnmapViewOfFile(_commAccessor);
	_commAccessor = nullptr;
	if (_commSharedMem) CloseHandle(_commSharedMem);
	_commSharedMem = nullptr;
	CloseHandle(_commAccessLock);
	_commAccessLock = nullptr;

	if (_screenAccessor[0]) UnmapViewOfFile(_screenAccessor[0]);
	if (_screenAccessor[1]) UnmapViewOfFile(_screenAccessor[1]);
	_screenAccessor[0] = nullptr; _screenAccessor[1] = nullptr;
	if (_screenSharedMem[0]) CloseHandle(_screenSharedMem[0]);
	if (_screenSharedMem[1]) CloseHandle(_screenSharedMem[1]);
	_screenSharedMem[0] = nullptr; _screenSharedMem[1] = nullptr;
	CloseHandle(_screenAccessLock[0]); CloseHandle(_screenAccessLock[1]);
	_screenAccessLock[0] = nullptr; _screenAccessLock[1] = nullptr;

}

CommState CShMem::GetState()
{
	if (WaitForSingleObject(_commAccessLock, 100) == WAIT_OBJECT_0)
	{
		_status = *((CommStatus*)&_commAccessor[_statusOffset]);
		WatchDogEnabled = _status.EnableWatchDog ? true : false;
		ReleaseMutex(_commAccessLock);
	}	
	return _status.State;		
}

void CShMem::WriteOutgoingPacket(char *packet)
{
	if (WaitForSingleObject(_commAccessLock, 100) == WAIT_OBJECT_0)
	{
		_status = *((CommStatus*)&_commAccessor[_statusOffset]);
		_status.DriverTime = GetTickCount();
		if (_status.OutgoingPackets < _maxPackets)
		{
			int offset = _outgoingOffset + (_status.OutgoingPackets * _packetSize);
			for (int i = 0; i < _packetSize; i++)
				_commAccessor[offset + i] = packet[i];
			_status.OutgoingPackets++;
			*((CommStatus*)&_commAccessor[_statusOffset]) = _status;
		}
		else
		{
			//reset buffer
			OutputDebugString(L"Buffer full\n");
			_status.OutgoingPackets = 0;
			*((CommStatus*)&_commAccessor[_statusOffset]) = _status;
		}
		ReleaseMutex(_commAccessLock);
	}
}

char *CShMem::ReadIncomingPackets(int *count)
{
	*count = 0;
	char *result = nullptr;
	if (WaitForSingleObject(_commAccessLock, 100) == WAIT_OBJECT_0)
	{
		_status = *((CommStatus*)&_commAccessor[_statusOffset]);
		_status.DriverTime = GetTickCount();
		if (_status.IncomingPackets > 0)
		{
			result = new char[_status.IncomingPackets * _packetSize];
			int pos = 0;
			for (auto p = 0; p < _status.IncomingPackets; p++)
			{				
				auto currOffset = _incomingOffset + (p * _packetSize);
				for (auto i = 0; i < _packetSize; i++)
					result[pos++] = _commAccessor[currOffset + i];
			}
			*count = _status.IncomingPackets;
			_status.IncomingPackets = 0;			
		}
		*((CommStatus*)&_commAccessor[_statusOffset]) = _status;
		ReleaseMutex(_commAccessLock);
	}
	return result;
}

void CShMem::WriteScreen(int eye, char *screenData, int size)
{
	auto si = (ScreenInfo*)_screenAccessor[eye];	
	auto dd = _screenAccessor[eye] + sizeof(ScreenInfo);
	if (WaitForSingleObject(_screenAccessLock[eye], 100) == WAIT_OBJECT_0)
	{
		si->Size = size;		
		memcpy(dd, screenData, size);
		ReleaseMutex(_screenAccessLock[eye]);
	}
}
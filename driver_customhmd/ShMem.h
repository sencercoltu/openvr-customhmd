#pragma once
#include <windows.h>

#pragma pack(push)

enum CommState
{
	Disconnected = 0,
	Connected = 1,
	ActiveNoDriver = 2,
	Active = 3	
};

#pragma pack(1)

struct ScreenInfo
{
	unsigned int Size;
};

struct CommStatus
{
	CommState State;
	unsigned int DriverTime;
	int EnableWatchDog;
	int IncomingPackets;
	int OutgoingPackets;
};
#pragma pack(pop)



class CShMem
{
public:
	CShMem();
	~CShMem();
	CommStatus _status;
	void WriteScreen(char *screenData, int size);
	void WriteOutgoingPacket(char *packet);
	char* ReadIncomingPackets(int *count);
	CommState GetState();
	bool WatchDogEnabled;
private:

	const int _maxPackets = 16;
	const int _packetSize = 32;
	const int _statusOffset = 0;
	const int _incomingOffset = sizeof(CommStatus);
	const int _outgoingOffset = _incomingOffset + (_packetSize * _maxPackets);

	const int _commBufferSize = 1024 * 1024;
	HANDLE _commAccessLock;
	HANDLE _commSharedMem;
	char *_commAccessor;

	const int _screenBufferSize = sizeof(ScreenInfo) + (3840 * 2160 * 4); //ScreenInfo + 4K display uncompressed
	HANDLE _screenAccessLock;
	HANDLE _screenSharedMem;
	char *_screenAccessor;

};


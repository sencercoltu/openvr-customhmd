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
	void WriteOutgoingPacket(char *packet);
	char* ReadIncomingPackets(int *count);
	CommState GetState();
	bool WatchDogEnabled;
private:
	const int _bufferSize = 1024 * 1024;
	const int _maxPackets = 16;
	const int _packetSize = 32;
	const int _statusOffset = 0;
	const int _incomingOffset = sizeof(CommStatus);
	const int _outgoingOffset = _incomingOffset + (_packetSize * _maxPackets);
	HANDLE _accessLock;
	HANDLE _sharedMem;
	char *_accessor;	

};


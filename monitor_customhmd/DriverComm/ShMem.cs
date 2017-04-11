using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Threading;

namespace monitor_customhmd.DriverComm
{
    public enum CommState
    {
        Disconnected = 0,
        Connected = 1,
        ActiveNoDriver = 2,
        Active = 3
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
    public struct ScreenInfo
    {
        public int Size;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
    public struct ScreenPartInfo
    {
        public int Eye;
        public int Size;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
    public struct CommStatus
    {
        public CommState State;
        public uint DriverTime;
        public int EnableWatchDog;
        public int IncomingPackets;
        public int OutgoingPackets;
    };

    public class ShMem : IDisposable
    {
        private static int _screenBufferSize = Marshal.SizeOf(typeof(ScreenInfo)) + (3840 * 2160 * 4); //ScreenInfo + 4K display
        private static int _commBufferSize = 1024 * 1024;
        private static int _statusSize = Marshal.SizeOf(typeof(CommStatus));
        private static int _packetSize = 32;
        private static int _maxPackets = 16;
        private static int _statusOffset = 0;
        private static int _incomingOffset = _statusSize;
        private static int _outgoingOffset = _incomingOffset + (_packetSize * _maxPackets);
        private uint _driverTimestamp = 0;
        private uint _prevDriverTimestamp = 0;

        private DateTime DriverTime = DateTime.MinValue;

        private CommStatus _status;
        public CommStatus Status { get { return _status; } }



        private Mutex _commAccessLock;
        private MemoryMappedFile _commSharedMem;
        private MemoryMappedViewAccessor _commAccessor;

        private Mutex[] _screenAccessLock = new Mutex[2];        
        private MemoryMappedFile[] _screenSharedMem = new MemoryMappedFile[2];         
        private MemoryMappedViewAccessor[] _screenAccessor = new MemoryMappedViewAccessor[2];

        public ShMem()
        {
            var newMutex = false;
            var mutexSecurity = new MutexSecurity(); mutexSecurity.AddAccessRule(new MutexAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), MutexRights.FullControl, AccessControlType.Allow));
            var memSecurity = new MemoryMappedFileSecurity(); memSecurity.AddAccessRule(new AccessRule<MemoryMappedFileRights>(new SecurityIdentifier(WellKnownSidType.WorldSid, null), MemoryMappedFileRights.FullControl, AccessControlType.Allow));

            _commAccessLock = new Mutex(false, "Global\\CustomHMDCommLock", out newMutex, mutexSecurity);
            _commSharedMem = MemoryMappedFile.CreateOrOpen("CustomHMDComm", _commBufferSize, MemoryMappedFileAccess.ReadWrite, MemoryMappedFileOptions.None, memSecurity, System.IO.HandleInheritability.Inheritable);
            _commAccessor = _commSharedMem.CreateViewAccessor();

            _screenAccessLock[0] = new Mutex(false, "Global\\CustomHMDLeftLock", out newMutex, mutexSecurity);
            _screenSharedMem[0] = MemoryMappedFile.CreateOrOpen("CustomHMDLeft", _screenBufferSize, MemoryMappedFileAccess.ReadWrite, MemoryMappedFileOptions.None, memSecurity, System.IO.HandleInheritability.Inheritable);
            _screenAccessor[0] = _screenSharedMem[0].CreateViewAccessor();

            _screenAccessLock[1] = new Mutex(false, "Global\\CustomHMDRightLock", out newMutex, mutexSecurity);
            _screenSharedMem[1] = MemoryMappedFile.CreateOrOpen("CustomHMDRight", _screenBufferSize, MemoryMappedFileAccess.ReadWrite, MemoryMappedFileOptions.None, memSecurity, System.IO.HandleInheritability.Inheritable);
            _screenAccessor[1] = _screenSharedMem[1].CreateViewAccessor();

        }

        public void Dispose()
        {
            SetState(CommState.Disconnected);

            _commAccessor.Dispose();
            _commAccessor = null;
            _commSharedMem.Dispose();
            _commSharedMem = null;
            _commAccessLock.Dispose();
            _commAccessLock = null;

            _screenAccessor[0].Dispose();
            _screenAccessor[0] = null;
            _screenSharedMem[0].Dispose();
            _screenSharedMem[0] = null;
            _screenAccessLock[0].Dispose();
            _screenAccessLock[0] = null;

            _screenAccessor[1].Dispose();
            _screenAccessor[1] = null;
            _screenSharedMem[1].Dispose();
            _screenSharedMem[1] = null;
            _screenAccessLock[1].Dispose();
            _screenAccessLock[1] = null;
        }




        public void SetState(CommState state)
        {
            if (_commAccessLock.WaitOne(100))
            {
                _commAccessor.Read(_statusOffset, out _status);
                _status.State = state;
                _driverTimestamp = _status.DriverTime;
                if (_driverTimestamp != _prevDriverTimestamp) DriverTime = DateTime.Now;
                _prevDriverTimestamp = _driverTimestamp;
                _commAccessor.Write(_statusOffset, ref _status);
                _commAccessLock.ReleaseMutex();
            }
        }

        public bool IsDriverActive
        {
            get
            {
                return (DateTime.Now - DriverTime).TotalSeconds < 2;
            }
        }

        public void EnableWatchDog(bool en)
        {
            if (_commAccessLock.WaitOne(100))
            {
                _commAccessor.Read(_statusOffset, out _status);
                _driverTimestamp = _status.DriverTime;
                if (_driverTimestamp != _prevDriverTimestamp) DriverTime = DateTime.Now;
                _prevDriverTimestamp = _driverTimestamp;
                _status.EnableWatchDog = en ? 1 : 0;
                _commAccessor.Write(_statusOffset, ref _status);
                _commAccessLock.ReleaseMutex();
            }
        }

        public void WriteIncomingPacket(byte[] packet)
        {
            if (_commAccessLock.WaitOne(100))
            {
                _commAccessor.Read(_statusOffset, out _status);
                _status.State = CommState.Active;
                _driverTimestamp = _status.DriverTime;
                if (_driverTimestamp != _prevDriverTimestamp) DriverTime = DateTime.Now;
                _prevDriverTimestamp = _driverTimestamp;
                if (_status.IncomingPackets < _maxPackets)
                {
                    var offset = _incomingOffset + (_status.IncomingPackets * _packetSize);
                    for (int i = 0; i < _packetSize; i++)
                        _commAccessor.Write(offset + i, ref packet[i + 1]);
                    _status.IncomingPackets++;
                    _commAccessor.Write(_statusOffset, ref _status);
                }
                else
                {
                    //reset buffer
                    Debug.WriteLine("Buffer full");
                    _status.IncomingPackets = 0;
                    _commAccessor.Write(_statusOffset, ref _status);
                }
                _commAccessLock.ReleaseMutex();
            }
        }

        public List<byte[]> ReadOutgoingPackets()
        {
            List<byte[]> result = null;
            if (_commAccessLock.WaitOne(100))
            {
                _commAccessor.Read(_statusOffset, out _status);
                _driverTimestamp = _status.DriverTime;
                if (_driverTimestamp != _prevDriverTimestamp) DriverTime = DateTime.Now;
                _prevDriverTimestamp = _driverTimestamp;
                if (_status.OutgoingPackets > 0)
                {
                    result = new List<byte[]>();
                    for (var p = 0; p < _status.OutgoingPackets; p++)
                    {
                        byte[] data = new byte[33];
                        var currOffset = _outgoingOffset + (p * _packetSize);
                        for (var i = 0; i < _packetSize; i++)
                            _commAccessor.Read(currOffset + i, out data[i + 1]);
                        result.Add(data);
                    }
                    _status.OutgoingPackets = 0;
                    _commAccessor.Write(_statusOffset, ref _status);
                }
                _commAccessLock.ReleaseMutex();
            }
            return result;
        }

        public int GetEyeImageSize(int eye)
        {
            if (!IsDriverActive)
                return 0;
            ScreenInfo info;
            int res = 0;
            if (_screenAccessLock[eye].WaitOne(10))
            {
                _screenAccessor[eye].Read(0, out info);
                res = info.Size;                                     
                info.Size = 0;
                _screenAccessor[eye].Write(0, ref info);
                _screenAccessLock[eye].ReleaseMutex();
            }            
            return res;
        }

        public void GetEyeImage(int eye, int size, out byte[] bitmap)
        {
            bitmap = null;            
            var infoSize = Marshal.SizeOf(typeof(ScreenInfo));
            if (_screenAccessLock[eye].WaitOne(10))
            {
                using (var stream = _screenSharedMem[eye].CreateViewStream(infoSize, size))
                {
                    using (BinaryReader binReader = new BinaryReader(stream))
                    {
                        bitmap = binReader.ReadBytes(size);
                        
                    }
                }
                _screenAccessLock[eye].ReleaseMutex();
            }
        }
    }
}

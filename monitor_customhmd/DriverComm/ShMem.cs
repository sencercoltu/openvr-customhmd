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
        public int Updated;
        public int Stride;
        public int Width;
        public int Height;
        public int Size() { return Stride * Height; }
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
        private Mutex _screenAccessLock;
        private MemoryMappedFile _commSharedMem;
        private MemoryMappedFile _screenSharedMem;
        private MemoryMappedViewAccessor _commAccessor;
        private MemoryMappedViewAccessor _screenAccessor;

        public ShMem()
        {
            var newMutex = false;
            var mutexSecurity = new MutexSecurity(); mutexSecurity.AddAccessRule(new MutexAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), MutexRights.FullControl, AccessControlType.Allow));
            var memSecurity = new MemoryMappedFileSecurity(); memSecurity.AddAccessRule(new AccessRule<MemoryMappedFileRights>(new SecurityIdentifier(WellKnownSidType.WorldSid, null), MemoryMappedFileRights.FullControl, AccessControlType.Allow));

            _commAccessLock = new Mutex(false, "Global\\CustomHMDCommLock", out newMutex, mutexSecurity);
            _commSharedMem = MemoryMappedFile.CreateOrOpen("CustomHMDComm", _commBufferSize, MemoryMappedFileAccess.ReadWrite, MemoryMappedFileOptions.None, memSecurity, System.IO.HandleInheritability.Inheritable);
            _commAccessor = _commSharedMem.CreateViewAccessor();

            _screenAccessLock = new Mutex(false, "Global\\CustomHMDDispLock", out newMutex, mutexSecurity);
            _screenSharedMem = MemoryMappedFile.CreateOrOpen("CustomHMDDisp", _screenBufferSize, MemoryMappedFileAccess.ReadWrite, MemoryMappedFileOptions.None, memSecurity, System.IO.HandleInheritability.Inheritable);
            _screenAccessor = _screenSharedMem.CreateViewAccessor();
        }

        public void Dispose()
        {
            SetState(CommState.Disconnected);

            _commAccessor.Dispose();
            _commAccessor = null;
            _commSharedMem.Dispose();
            _commSharedMem = null;

            _screenAccessor.Dispose();
            _screenAccessor = null;
            _screenSharedMem.Dispose();
            _screenSharedMem = null;

            _commAccessLock.Dispose();
            _commAccessLock = null;

            _screenAccessLock.Dispose();
            _screenAccessLock = null;
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

        public int GetScreenInfo(ref ScreenInfo info)
        {
            if (!IsDriverActive)
                return 0;
            int res = 0;
            if (_screenAccessLock.WaitOne(10))
            {
                _screenAccessor.Read(0, out info);
                res = info.Updated;                                     
                info.Updated = 0;
                _screenAccessor.Write(0, ref info);
                _screenAccessLock.ReleaseMutex();
            }            
            return res;
        }

        public void GetScreenBitmap(ref ScreenInfo info, out byte[] bmpLeft, out byte[] bmpRight)
        {
            bmpLeft = null;
            bmpRight = null;
            var size = info.Stride * info.Height;
            var infoSize = Marshal.SizeOf(typeof(ScreenInfo));
            if (_screenAccessLock.WaitOne(10))
            {
                using (var stream = _screenSharedMem.CreateViewStream(infoSize, infoSize + (size * 2)))
                {
                    using (BinaryReader binReader = new BinaryReader(stream))
                    {
                        bmpLeft = binReader.ReadBytes(size);
                        bmpRight = binReader.ReadBytes(size);
                    }
                }
                _screenAccessLock.ReleaseMutex();
            }
        }
    }
}

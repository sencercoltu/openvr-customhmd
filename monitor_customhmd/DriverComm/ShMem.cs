using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using static monitor_customhmd.UsbPacketDefs;

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
        private static int _bufferSize = 1024 * 1024;
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



        private Mutex _accessLock;
        private MemoryMappedFile _sharedMem;
        private MemoryMappedViewAccessor _accessor;

        public ShMem()
        {
            var newMutex = false;
            var mutexSecurity = new MutexSecurity(); mutexSecurity.AddAccessRule(new MutexAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), MutexRights.FullControl, AccessControlType.Allow));
            _accessLock = new Mutex(false, "Global\\CustomHMDCommLock", out newMutex, mutexSecurity);
            var memSecurity = new MemoryMappedFileSecurity(); memSecurity.AddAccessRule(new AccessRule<MemoryMappedFileRights>(new SecurityIdentifier(WellKnownSidType.WorldSid, null), MemoryMappedFileRights.FullControl, AccessControlType.Allow));
            _sharedMem = MemoryMappedFile.CreateOrOpen("CustomHMDComm", _bufferSize, MemoryMappedFileAccess.ReadWrite, MemoryMappedFileOptions.None, memSecurity, System.IO.HandleInheritability.Inheritable);
            _accessor = _sharedMem.CreateViewAccessor();
        }

        public void Dispose()
        {
            SetState(CommState.Disconnected);
            _accessor.Dispose();
            _accessor = null;
            _sharedMem.Dispose();
            _sharedMem = null;
            _accessLock.Dispose();
            _accessLock = null;
        }

        public void SetState(CommState state)
        {
            if (_accessLock.WaitOne(100))
            {
                _accessor.Read(_statusOffset, out _status);
                _status.State = state;
                _driverTimestamp = _status.DriverTime;
                if (_driverTimestamp != _prevDriverTimestamp) DriverTime = DateTime.Now;
                _accessor.Write(_statusOffset, ref _status);
                _accessLock.ReleaseMutex();
            }
        }

        public bool IsDriverActive
        {
            get
            {
                return (DateTime.Now - DriverTime).TotalSeconds < 5;
            }
        }

        public void EnableWatchDog(bool en)
        {
            if (_accessLock.WaitOne(100))
            {
                _accessor.Read(_statusOffset, out _status);
                _driverTimestamp = _status.DriverTime;
                if (_driverTimestamp != _prevDriverTimestamp) DriverTime = DateTime.Now;
                _status.EnableWatchDog = en ? 1 : 0;
                _accessor.Write(_statusOffset, ref _status);
                _accessLock.ReleaseMutex();
            }
        }

        public void WriteIncomingPacket(byte[] packet)
        {
            if (_accessLock.WaitOne(100))
            {
                _accessor.Read(_statusOffset, out _status);
                _status.State = CommState.Active;
                _driverTimestamp = _status.DriverTime;
                if (_driverTimestamp != _prevDriverTimestamp) DriverTime = DateTime.Now;
                if (_status.IncomingPackets < _maxPackets)
                {
                    var offset = _incomingOffset + (_status.IncomingPackets * _packetSize);
                    for (int i = 0; i < _packetSize; i++)
                        _accessor.Write(offset + i, ref packet[i + 1]);
                    _status.IncomingPackets++;
                    _accessor.Write(_statusOffset, ref _status);
                }
                else
                {
                    //reset buffer
                    Debug.WriteLine("Buffer full");
                    _status.IncomingPackets = 0;
                    _accessor.Write(_statusOffset, ref _status);
                }
                _accessLock.ReleaseMutex();
            }
        }

        public List<byte[]> ReadOutgoingPackets()
        {
            List<byte[]> result = null;
            if (_accessLock.WaitOne(100))
            {
                _accessor.Read(_statusOffset, out _status);
                _driverTimestamp = _status.DriverTime;
                if (_driverTimestamp != _prevDriverTimestamp) DriverTime = DateTime.Now;
                if (_status.OutgoingPackets > 0)
                {
                    result = new List<byte[]>();
                    for (var p = 0; p < _status.OutgoingPackets; p++)
                    {
                        byte[] data = new byte[33];
                        var currOffset = _outgoingOffset + (p * _packetSize);
                        for (var i = 0; i < _packetSize; i++)
                            _accessor.Read(currOffset + i, out data[i + 1]);
                        result.Add(data);
                    }
                    _status.OutgoingPackets = 0;
                    _accessor.Write(_statusOffset, ref _status);
                }
                _accessLock.ReleaseMutex();
            }
            return result;
        }
    }
}

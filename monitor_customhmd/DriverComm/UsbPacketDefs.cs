using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace monitor_customhmd
{
    public enum RawModes
    {
        Off,
        Raw,
        Filtered,        
        Compensated,
        DRVectors
    }

    public static class UsbPacketDefs
    {
        public const byte HMD_SOURCE = 0x00;
        public const byte LEFTCTL_SOURCE = 0x01;
        public const byte RIGHTCTL_SOURCE = 0x02;
        public const byte BASESTATION_SOURCE = 0x03;
        public const byte LIGHTHOUSE1_SOURCE = 0x04;
        public const byte LIGHTHOUSE2_SOURCE = 0x05;
        public const byte MAX_SOURCE = 0x06;
        public const byte INVALID_SOURCE = 0x0F;

        public const byte ROTATION_DATA = 0x10;
        public const byte POSITION_DATA = 0x20;
        public const byte TRIGGER_DATA = 0x40;
        public const byte COMMAND_DATA = 0x80;
        public const byte INVALID_DATA = 0xF0;

        public const byte CMD_NONE = 0x00;
        public const byte CMD_VIBRATE = 0x01;
        public const byte CMD_CALIBRATE = 0x02;
        public const byte CMD_SYNC = 0x03;
        public const byte CMD_RAW_DATA = 0x04;
        public const byte CMD_STATUS = 0x05;

        public const byte CALIB_SET = 0x00;
        public const byte CALIB_GET = 0x01;

        public const byte BUTTON_0 = 0x01;
        public const byte BUTTON_1 = 0x02;
        public const byte BUTTON_2 = 0x04;
        public const byte BUTTON_3 = 0x08;
        public const byte BUTTON_4 = 0x10;
        public const byte BUTTON_5 = 0x20;
        public const byte BUTTON_6 = 0x40;
        public const byte BUTTON_7 = 0x80;

        public const byte SENSOR_NONE = 0x00;
        public const byte SENSOR_ACCEL = 0x01;
        public const byte SENSOR_GYRO = 0x02;
        public const byte SENSOR_MAG = 0x03;
        //public const byte SENSOR_POSACCEL = 0x08;


        public const int CUSTOM_HID_EPIN_SIZE = 32;
        public const int CUSTOM_HID_EPOUT_SIZE = 32;
        public const int USB_CUSTOM_HID_DESC_SIZ = 34;

        public interface IUSBData { }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBDataHeader
        {
            public static int Size { get { return 4; } }
            public byte Type; //source & data 1
            public ushort Sequence; //source & data 2
            public byte Crc8; //source & data 1
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBPositionData : IUSBData
        {
            public static int Size { get { return 12; } }
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.R4)]
            public float[] Position;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBSyncData : IUSBData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBSyncData)); } }
            public ulong SyncTime;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBRotationData : IUSBData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBRotationData)); } }
            public float w;
            public float x;
            public float y;
            public float z;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBAxisData : IUSBData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBAxisData)); } }
            public float x;
            public float y;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBRawData : IUSBData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBRawData)); } }
            public byte State; //1 for enable, 0 for disable (set by driver)
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] Accel;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] Gyro;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] Mag;
            public void Init()
            {
                Accel = new short[3];
                Gyro = new short[3];
                Mag = new short[3];
            }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBTriggerData : IUSBData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBTriggerData)); } }
            public ushort Digital;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2, ArraySubType = UnmanagedType.Struct)]
            public USBAxisData[] Analog;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBCalibrationData : IUSBData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBCalibrationData)); } }
            public byte Command;
            public byte Sensor;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] RawMax;
            //[MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.U2)]
            //public ushort[] ScaleAccel;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] RawMin;
            public short Reserved;
            public void Init()
            {
                RawMax = new short[3];
                RawMin = new short[3];
            }
            //[MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            //public short[] OffsetMag;
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBVibrationData : IUSBData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBVibrationData)); } }
            public uint Axis;
            public ushort Duration;
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBStatusData : IUSBData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBStatusData)); } }
            public Int32 HeartBeat;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBCommandData : IUSBData
        {
            public static int Size { get { return 26; } }
            public byte Command;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 25, ArraySubType = UnmanagedType.U1)]
            public byte[] Data;

            public static USBCommandData Create(byte command, IUSBData packet)
            {
                var p = new USBCommandData
                {
                    Command = command,
                    Data = new byte[25]
                };
                var d = StructToBytes(packet);
                Array.Copy(d, p.Data, Math.Min(d.Length, 25));
                return p;
            }

            public void ParseFields()
            {
                switch (Command)
                {
                    case CMD_VIBRATE:
                        Vibration = StructFromBytes<USBVibrationData>(Data);
                        break;
                    case CMD_CALIBRATE:
                        Calibration = StructFromBytes<USBCalibrationData>(Data);
                        break;
                    case CMD_SYNC:
                        Sync = StructFromBytes<USBSyncData>(Data);
                        break;
                    case CMD_RAW_DATA:
                        Raw = StructFromBytes<USBRawData>(Data);
                        break;
                    case CMD_STATUS:
                        Status = StructFromBytes<USBStatusData>(Data);
                        break;
                }
            }

            public USBVibrationData Vibration;
            public USBSyncData Sync;
            public USBCalibrationData Calibration;
            public USBRawData Raw;
            public USBStatusData Status;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBDataCache //used at ps side/server only
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBDataCache)); } }
            public USBPositionData Position;
            public USBRotationData Rotation;
            public USBTriggerData Trigger;
        }


        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBPacket
        {
            public USBDataHeader Header;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 28, ArraySubType = UnmanagedType.U1)]
            private byte[] Data;
            public static USBPacket Create(byte type, ushort sequence, IUSBData packet)
            {
                var p = new USBPacket
                {
                    Header = {
                        Sequence = sequence,
                        Type = type
                    },
                    Data = new byte[28]
                    //Reserved = new byte[8],
                };
                var d = StructToBytes(packet);
                Array.Copy(d, p.Data, Math.Min(d.Length, 28)); // 20 data + 8 reserved
                return p;
            }
            public static int Size { get { return 32; } }
            public void ParseFields()
            {
                switch (Header.Type & 0xf0)
                {
                    case ROTATION_DATA:
                        Rotation = StructFromBytes<USBRotationData>(Data);
                        break;
                    case POSITION_DATA:
                        Position = StructFromBytes<USBPositionData>(Data);
                        break;
                    case TRIGGER_DATA:
                        Trigger = StructFromBytes<USBTriggerData>(Data);
                        break;
                    case COMMAND_DATA:
                        Command = StructFromBytes<USBCommandData>(Data);
                        Command.ParseFields();
                        break;
                }
            }
            public USBRotationData Rotation;
            public USBPositionData Position;
            public USBTriggerData Trigger;
            public USBCommandData Command;
        }

        public static void SetPacketCrc(ref byte[] data)
        {
            data[3] = 0;
            byte crc = 0;
            for (int i = 0; i < USBPacket.Size; i++)
                crc ^= data[i];
            data[3] = crc;
            var d = new byte[USBPacket.Size + 1];
            Array.Copy(data, 0, d, 1, USBPacket.Size);
            data = d;
        }

        public static byte GetPacketCrc(byte[] data, int idx)
        {
            byte crcTemp = data[3];
            data[3] = 0;
            byte crc = 0;
            for (int i = idx; i < idx + USBPacket.Size; i++)
                crc ^= data[i];
            data[3] = crcTemp;
            return crc;
        }

        public static bool CheckPacketCrc(byte[] data, int idx)
        {
            return data[3] == GetPacketCrc(data, idx);
        }

        public static ushort TO_CALIB_SCALE(float x)
        {
            return (ushort)(x * 32768.0f);
        }

        public static float FROM_CALIB_SCALE(ushort x)
        {
            return x / 32768.0f;
        }

        public static T StructFromBytes<T>(byte[] bytes, int index = 0)
        {
            var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            var ret = (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject() + index, typeof(T));
            handle.Free();
            return ret;
        }

        public static byte[] StructToBytes(object obj)
        {
            var packetSize = Marshal.SizeOf(obj);
            var packet = new byte[packetSize];
            var packetHandle = GCHandle.Alloc(packet, GCHandleType.Pinned);
            Marshal.StructureToPtr(obj, packetHandle.AddrOfPinnedObject(), false);
            packetHandle.Free();
            return packet;
        }


    }


}

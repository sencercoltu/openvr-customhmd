using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace monitor_customhmd
{
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

        public const byte BUTTON_0 = 0x01;
        public const byte BUTTON_1 = 0x02;
        public const byte BUTTON_2 = 0x04;
        public const byte BUTTON_3 = 0x08;

        public const byte SENSOR_ACCEL = 0x01;
        public const byte SENSOR_GYRO = 0x02;
        public const byte SENSOR_MAG = 0x04;


        public const int CUSTOM_HID_EPIN_SIZE = 32;
        public const int CUSTOM_HID_EPOUT_SIZE = 32;
        public const int USB_CUSTOM_HID_DESC_SIZ = 34;


        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBDataHeader
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBDataHeader)); } }
            public byte Type; //source & data
            public ushort Sequence; //source & data
            public byte Crc8; //source & data
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBPositionData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBPositionData)); } }
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.R4)]
            public float[] Position;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBSyncData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBSyncData)); } }
            public ulong SyncTime;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBRotationData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBRotationData)); } }
            public float w;
            public float x;
            public float y;
            public float z;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBAxisData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBAxisData)); } }
            public float x;
            public float y;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBRawData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBRawData)); } }
            public byte State; //1 for enable, 0 for disable (set by driver)
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] Accel;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] Gyro;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] Mag;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBTriggerData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBTriggerData)); } }
            public ushort Digital;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.Struct)]
            public USBAxisData[] Analog;
        }

        // 2/65535 ( 0.00003052 to 2.0)
        public static ushort TO_CALIB_SCALE(float x)
        {
            return (ushort)(x * 32768.0f);
        }

        public static float FROM_CALIB_SCALE(ushort x)
        {
            return x / 32768.0f;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBCalibrationData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBCalibrationData)); } }
            public byte SensorMask;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] OffsetAccel;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.U2)]
            public ushort[] ScaleAccel;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] OffsetGyro;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
            public short[] OffsetMag;
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBVibrationData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBVibrationData)); } }
            public uint Axis;
            public ushort Duration;
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBStatusData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBStatusData)); } }
            public byte CalibrationMask;
        }


        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBCommandData
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBCommandData)); } }
            public byte Command;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 25, ArraySubType = UnmanagedType.U1)]
            public byte[] Data;

            private USBVibrationData? _vibration; //6
            public USBVibrationData? Vibration { get { return _vibration.HasValue ? _vibration : (Command == 0x01) ? (USBVibrationData?)null : StructFromBytes<USBVibrationData>(Data); } }

            private USBSyncData? _sync; //8
            public USBSyncData? Sync { get { return _sync.HasValue ? _sync : (Command == 0x02) ? (USBSyncData?)null : StructFromBytes<USBSyncData>(Data); } }

            private USBCalibrationData? _calibration; //25
            public USBCalibrationData? Calibration { get { return _calibration.HasValue ? _calibration.Value : (Command == 0x03) ? (USBCalibrationData?)null : StructFromBytes<USBCalibrationData>(Data); } }

            private USBRawData? _raw; //19
            public USBRawData? Raw { get { return _raw.HasValue ? _raw : (Command == 0x04) ? (USBRawData?)null : StructFromBytes<USBRawData>(Data); } }

            private USBStatusData? _status; //1
            public USBStatusData? Status { get { return _status.HasValue ? _status : (Command == 0x05) ? (USBStatusData?)null : StructFromBytes<USBStatusData>(Data); } }
        }

        //[StructLayout(LayoutKind.Explicit, CharSet = CharSet.Ansi, Pack = 1)]
        //public struct USBData
        //{
        //    public static int Size { get { return Marshal.SizeOf(typeof(USBData)); } }
        //    private byte[] Data;

        //    [FieldOffset(0)]
        //    public USBPositionData Position;
        //    [FieldOffset(0)]
        //    public USBRotationData Rotation;
        //    [FieldOffset(0)]
        //    public USBTriggerData Trigger;
        //    [FieldOffset(0)]
        //    public USBCommandData Command;
        //}

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBDataCache //used at ps side/server only
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBDataCache)); } }
            public USBPositionData Position;
            public USBRotationData Rotation;
            public USBTriggerData Trigger;
        }


        //private const int reservedSize = 1; // UsbPacket.CUSTOM_HID_EPOUT_SIZE - (Marshal.SizeOf(typeof(USBData)) + Marshal.SizeOf(typeof(USBDataHeader)));

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct USBPacket
        {
            public static int Size { get { return Marshal.SizeOf(typeof(USBPacket)); } }
            public USBDataHeader Header;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 20, ArraySubType = UnmanagedType.U1)]
            private byte[] Data;

            private USBPositionData? _position { get; set; }            
            public USBPositionData? Position {  get { return _position.HasValue? _position : ((Header.Type & 0xf0) != 0x10)? (USBPositionData?) null :  StructFromBytes<USBPositionData>(Data); } }

            private USBRotationData? _rotation { get; set; }
            public USBRotationData? Rotation { get { return _rotation.HasValue ? _rotation : ((Header.Type & 0xf0) != 0x20) ? (USBRotationData?)null : StructFromBytes<USBRotationData>(Data); } }

            private USBTriggerData? _trigger { get; set; }
            public USBTriggerData? Trigger { get { return _trigger.HasValue ? _trigger : ((Header.Type & 0xf0) != 0x40) ? (USBTriggerData?)null : StructFromBytes<USBTriggerData>(Data); } }

            private USBCommandData? _command { get; set; }
            public USBCommandData? Command { get { return _command.HasValue ? _command : ((Header.Type & 0xf0) != 0x80) ? (USBCommandData?)null : StructFromBytes<USBCommandData>(Data); } }


            //public USBData Data;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8, ArraySubType = UnmanagedType.U1)]
            private byte[] Reserved;
        }

        public static void SetPacketCrc(ref USBPacket pPacket) { }
        public static byte GetPacketCrc(ref USBPacket pPacket) { return 0; }
        public static byte CheckPacketCrc(ref USBPacket pPacket) { return 0; }

        public static T StructFromBytes<T>(byte[] bytes, int index = 0)
        {
            var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            var ret = (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject()+index, typeof(T));
            handle.Free();
            return ret;
        }

        public static byte[] StructToBytes(object obj)
        {
            var packetSize = Marshal.SizeOf(obj.GetType());
            var packet = new byte[packetSize];
            var packetHandle = GCHandle.Alloc(packet, GCHandleType.Pinned);
            Marshal.StructureToPtr(obj, packetHandle.AddrOfPinnedObject(), false);
            packetHandle.Free();
            return packet;
        }

    }


}

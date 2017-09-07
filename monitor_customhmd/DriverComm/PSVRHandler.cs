using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using LibUsbDotNet;
using LibUsbDotNet.Main;
using LibUsbDotNet.WinUsb;
using static monitor_customhmd.UsbPacketDefs;

namespace monitor_customhmd.DriverComm
{
    enum PSVRReportType : byte
    {
        None = 0x00,
        Tracking = 0x11,
        Power = 0x13,
        Led = 0x15,
        Display = 0x17,
        CinematicConf = 0x21,
        Mode = 0x23,
        Info = 0x81,
        Result = 0xA0,
        Status = 0xF0,
    }

    [Flags]
    enum PSVRLed : ushort
    {
        None = 0x0000,
        A = 0x0001,
        B = 0x0002,
        C = 0x0004,
        D = 0x0008,
        E = 0x0010,
        F = 0x0020,
        G = 0x0040,
        H = 0x0080,
        I = 0x0100,
        All = A | B | C | D | E | F | G | H | I
    }

    class PSVRReport
    {
        public PSVRReportType ReportType { get; private set; } = PSVRReportType.None;

        public readonly byte[] Data = new byte[64];

        private PSVRReport()
        {

        }

        public PSVRReport(PSVRReportType reportType, byte[] data)
        {
            ReportType = reportType;
            Data[0] = (byte)ReportType;
            Data[1] = 0x00;
            Data[2] = 0xaa;
            Data[3] = (byte)data.Length;
            Array.Copy(data, 0, Data, 4, data.Length);
            Debug.WriteLine("Created request type: {0}", reportType);
        }

        public static PSVRReport Parse(byte[] data)
        {
            var report = new PSVRReport();
            report.ReportType = (PSVRReportType)data[0];
            Array.Copy(data, 0, report.Data, 0, data.Length);
            return report;
        }
    }


    [Flags]
    public enum ControlButtons
    {
        None = 0,
        VolumeUp = 2,
        VolumeDown = 4,
        Mute = 8
    }

    class PSVRHandler : IDisposable
    {
        public bool IsWorn { get; private set; }
        public bool IsDisplayActive { get; private set; }
        public bool IsMuted { get; private set; }
        public bool IsEarphoneConnected { get; private set; }
        public bool IsCalibrated { get; private set; }
        public bool IsReady { get; private set; }
        public byte Volume { get; private set; }
        public byte Proximity1 { get; private set; }
        public byte Proximity2 { get; private set; }

        public Action<byte[], USBPacket?> OnPacketReceive = null;

        private UsbEndpointReader _sensorRead = null;
        private UsbEndpointReader _controlRead = null;
        private UsbEndpointWriter _controlWrite = null;

        public UInt32 LastTimeStamp { get; private set; }
        private float[] Accel = new float[3];
        private float[] Gyro = new float[3];

        public bool IsActive { get { return OpenDevice(); } }

        public PSVRHandler()
        {
        }

        private bool OpenDevice()
        {
            if (_controlWrite == null)
            {
                var device = UsbDevice.AllWinUsbDevices.FirstOrDefault(d =>
                {
                    var dev = d as WinUsbRegistry;
                    return dev.Vid == 0x054C && dev.Pid == 0x09AF && dev.InterfaceID == 5;
                }) as WinUsbRegistry;
                if (device != null)
                {
                    WinUsbDevice _usb;
                    if (device.Open(out _usb))
                    {
                        PostActions.Clear();

                        _controlRead = _usb.OpenEndpointReader(ReadEndpointID.Ep04);
                        _controlRead.Reset();
                        _controlRead.Flush();
                        _controlRead.ReadBufferSize = 64;
                        _controlRead.DataReceived += ControlRead;
                        _controlRead.DataReceivedEnabled = true;

                        _controlWrite = _usb.OpenEndpointWriter(WriteEndpointID.Ep04);
                        _controlWrite.Reset();
                        _controlWrite.Flush();

                        //ActivateDisplay(true, () =>
                        //{
                        //    LedControl(true, () =>
                        //    {
                        //        EnableTracking(true, () =>
                        //        {
                        //            SetVRMode(true);
                        //        });
                        //    });
                        //});
                    }
                }
            }

            if (_sensorRead == null)
            {
                var device = UsbDevice.AllWinUsbDevices.FirstOrDefault(d =>
                {
                    var dev = d as WinUsbRegistry;
                    return dev.Vid == 0x054C && dev.Pid == 0x09AF && dev.InterfaceID == 4;
                }) as WinUsbRegistry;
                if (device != null)
                {
                    WinUsbDevice _usb;
                    if (device.Open(out _usb))
                    {
                        _sensorRead = _usb.OpenEndpointReader(ReadEndpointID.Ep03);
                        _sensorRead.Reset();
                        _sensorRead.Flush();
                        _sensorRead.ReadBufferSize = 64;
                        _sensorRead.DataReceived += SensorRead;
                        _sensorRead.DataReceivedEnabled = true;
                    }
                }
            }

            return _sensorRead != null && _controlWrite != null;
        }

        private void CloseDevice()
        {            
            if (_sensorRead != null)
            {
                _sensorRead.DataReceivedEnabled = false;
                _sensorRead.DataReceived -= SensorRead;
                _sensorRead.Device.Close();
                _sensorRead.Dispose();
            }
            _sensorRead = null;

            if (_controlRead != null)
            {
                _controlRead.DataReceivedEnabled = false;
                _controlRead.DataReceived -= ControlRead;
                _controlRead.Dispose();
            }
            _controlRead = null;

            if (_controlWrite != null)
            {
                _controlWrite.Dispose();
                _controlWrite.Device.Close();
            }
            _controlWrite = null;
            UsbDevice.Exit();
            PostActions.Clear();
        }

        private void SensorRead(object sender, EndpointDataEventArgs e)
        {
            var data = e.Buffer;
            var pressedButton = (ControlButtons)data[0];

            Volume = data[2];
            var status = data[8];
            IsWorn = (status & 0x1) == 0x1 ? true : false;
            IsDisplayActive = (status & 0x2) == 0x2 ? false : true;
            IsMuted = (status & 0x8) == 0x8 ? true : false;
            IsEarphoneConnected = (status & 0x10) == 0x10 ? true : false;

            Proximity1 = data[39];
            Proximity2 = data[40];

            //2 sets of sensor data per packet?
            var pos = 16;
            var timeStamp = BitConverter.ToUInt32(data, pos);

            Gyro[0] = BitConverter.ToInt16(data, pos + 4);
            Gyro[1] = BitConverter.ToInt16(data, pos + 6);
            Gyro[2] = BitConverter.ToInt16(data, pos + 8);

            Accel[0] = BitConverter.ToInt16(data, pos + 10);
            Accel[1] = BitConverter.ToInt16(data, pos + 12);
            Accel[2] = BitConverter.ToInt16(data, pos + 14);

            LastTimeStamp = timeStamp;

            //process

            pos += 16;
            timeStamp = BitConverter.ToUInt32(data, pos);

            Gyro[0] = BitConverter.ToInt16(data, pos + 4);
            Gyro[1] = BitConverter.ToInt16(data, pos + 6);
            Gyro[2] = BitConverter.ToInt16(data, pos + 8);

            Accel[0] = BitConverter.ToInt16(data, pos + 10);
            Accel[1] = BitConverter.ToInt16(data, pos + 12);
            Accel[2] = BitConverter.ToInt16(data, pos + 14);

            LastTimeStamp = timeStamp;

            //process            

            OnPacketReceive?.Invoke(null, null);
        }

        private Dictionary<PSVRReportType, Action> PostActions = new Dictionary<PSVRReportType, Action>();

        private void ControlRead(object sender, EndpointDataEventArgs e)
        {
            var report = PSVRReport.Parse(e.Buffer);
            Debug.WriteLine("Report: " + report.ReportType);
            switch (report.ReportType)
            {
                case PSVRReportType.Info:
                    //result for 
                    break;
                case PSVRReportType.Status:
                    //hmd status
                    break;
                case PSVRReportType.Result:
                    //result for command
                    var type = (PSVRReportType)report.Data[4];
                    Debug.WriteLine("Got response to request: {0}", type);
                    Action action;
                    PostActions.TryGetValue(type, out action);
                    if (action != null)
                    {
                        PostActions.Remove(type);
                        ThreadPool.QueueUserWorkItem((a) => { ((Action)a).Invoke(); }, action);                        
                    }
                    break;
            }
        }

        internal void ProcessOutgoing(USBPacket packet)
        {
            //process the data...
        }

        public void Dispose()
        {
            CloseDevice();
            return;

            //PowerOffHeadset();
            LedControl(false, () => 
            {
                ActivateDisplay(false, CloseDevice);
            });
        }

        public void EnableTracking(bool en, Action postAction = null)
        {
            if (!IsActive)
                return;
            var report = new PSVRReport(PSVRReportType.Tracking, new byte[] { (byte)(en ? 0x80 : 0x00), 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 });
            if (postAction != null)
                PostActions[report.ReportType] = postAction;
            int len;
            _controlWrite.Write(report.Data, 1000, out len);
        }

        public void ActivateDisplay(bool en, Action postAction = null)
        {
            if (!IsActive)
                return;
            var report = new PSVRReport(PSVRReportType.Display, BitConverter.GetBytes(en ? 0x00000001 : 0x00000000));
            if (postAction != null)
                PostActions[report.ReportType] = postAction;
            int len;
            _controlWrite.Write(report.Data, 1000, out len);
        }

        public void SetVRMode(bool en, Action postAction = null)
        {
            if (!IsActive)
                return;
            var report = new PSVRReport(PSVRReportType.Mode, BitConverter.GetBytes(en ? 0x00000001 : 0x00000000));
            if (postAction != null)
                PostActions[report.ReportType] = postAction;
            int len;
            _controlWrite.Write(report.Data, 1000, out len);
        }

        public void PowerOffHeadset()
        {
            if (_controlWrite == null)
                return;
            var report = new PSVRReport(PSVRReportType.Power, BitConverter.GetBytes(0x00000001));
            int len;
            _controlWrite.Write(report.Data, 1000, out len);
        }

        public void LedControl(bool en, Action postAction = null)
        {
            if (!IsActive)
                return;
            byte val = (byte)(en ? 0x64 : 0x00);
            var report = new PSVRReport(PSVRReportType.Led, new byte[] { (ushort)PSVRLed.All & 0xFF, ((ushort)PSVRLed.All >> 8) & 0xFF, val, val, val, val, val, val, val, val, val, 0, 0, 0, 0, 0 });
            if (postAction != null)
                PostActions[report.ReportType] = postAction;
            int len;
            _controlWrite.Write(report.Data, 1000, out len);
        }

        public void SetCinematicConfig(byte Mask, byte ScreenDistance, byte ScreenSize, byte IPD, byte Brightness, byte MicVolume, bool UnknownVRSetting)
        {
            //var report = new PSVRReport();

            //cmd.ReportID = 0x21;
            //cmd.DataStart = 0xaa;
            //cmd.DataLength = 16;
            //cmd.Data = new byte[] { Mask, ScreenSize, ScreenDistance, IPD, 0, 0, 0, 0, 0, 0, Brightness, MicVolume, 0, 0, (byte)(UnknownVRSetting ? 0 : 1), 0 };

            
        }
    }
}

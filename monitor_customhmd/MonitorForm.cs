using HidLibrary;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using Valve.VR;
using static monitor_customhmd.UsbPacketDefs;

namespace monitor_customhmd
{
    public partial class MonitorForm : Form
    {
        private CVRSystem _vrSystem;
        
        private Thread _thread;
        private bool _running;

        private Queue<USBPacket> _incoming = new Queue<USBPacket>();
        private Queue<USBPacket> _outgoing = new Queue<USBPacket>();

        public MonitorForm()
        {
            InitializeComponent();                 
        }

        private void MonitorForm_Load(object sender, EventArgs e)
        {
            var err = EVRInitError.Driver_Failed;
            _vrSystem = OpenVR.Init(ref err, EVRApplicationType.VRApplication_Overlay);
            OpenVR.GetGenericInterface(OpenVR.IVRCompositor_Version, ref err);
            OpenVR.GetGenericInterface(OpenVR.IVROverlay_Version, ref err);

            _thread = new Thread(() =>
            {
                HidDevice _usb = null;

                while (_running)
                {
                    if (_usb == null)
                        _usb = HidDevices.Enumerate(0x1974, new[] { 0x001 }).FirstOrDefault();

                    if (_usb != null)
                    {
                        if (!_usb.IsConnected)
                        {
                            _usb.Dispose();
                            _usb = null;
                            Thread.Sleep(100);
                            continue;
                        }
                        if (!_usb.IsOpen)
                        {
                            _usb.OpenDevice(DeviceMode.Overlapped, DeviceMode.Overlapped, ShareMode.ShareRead | ShareMode.ShareWrite);
                            Thread.Sleep(10);
                            continue;
                        }
                        if (_usb.IsOpen)
                        {
                            lock(_outgoing)
                                if (_outgoing.Count > 0)
                                {
                                    var p = _outgoing.Dequeue();
                                    var d = StructToBytes(p);
                                    _usb.Write(d);
                                }
                            var data = _usb.Read(10);
                            if (data.Status == HidDeviceData.ReadStatus.Success)
                            {
                                var packet = StructFromBytes<USBPacket>(data.Data, 1);
                                lock (_incoming)
                                    _incoming.Enqueue(packet);
                            }
                            else
                                Thread.Sleep(1);
                        }
                        else
                            Thread.Sleep(100);
                    }
                }
            }) { IsBackground = true};
            _running = true;
            _thread.Start();
        }

        ulong ulMainHandle;

        private void button1_Click(object sender, EventArgs e)
        {
            var overlay = OpenVR.Overlay;

            if (ulMainHandle != 0)
                overlay.DestroyOverlay(ulMainHandle);
            var err = overlay.CreateOverlay("TEST", "HL3", ref ulMainHandle);
            //err = overlay.SetOverlayTransformTrackedDeviceRelative(ulMainHandle, OpenVR.k_unTrackedDeviceIndex_Hmd, &m_overlayDistanceMtx);
            err = overlay.SetOverlayFromFile(ulMainHandle, @".\hl3.jpg");
            err = overlay.SetOverlayWidthInMeters(ulMainHandle, 2.5f);
            err = overlay.SetOverlayInputMethod(ulMainHandle, VROverlayInputMethod.Mouse);            
            err = overlay.ShowOverlay(ulMainHandle);
        }

        private void MonitorForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            _running = false;
            _thread.Join();
            _thread = null;

            if (ulMainHandle != 0)
                OpenVR.Overlay.DestroyOverlay(ulMainHandle);
            OpenVR.Shutdown();
        }

        private void tmrConsumer_Tick(object sender, EventArgs e)
        {
            bool done = false;
            USBPacket packet = default(USBPacket);

            while (!done)
            {                
                lock (_incoming)
                {
                    done = _incoming.Count == 0;
                    if (!done)
                    {
                        packet = _incoming.Dequeue();
                    }
                }
                if (!done)
                    OnReceivePacket(ref packet);
            }
        }

        private void OnReceivePacket(ref USBPacket packet)
        {
            lblSequence.Text = packet.Header.Sequence.ToString();
        }
    }
}

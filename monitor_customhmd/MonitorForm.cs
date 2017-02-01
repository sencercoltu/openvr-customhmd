using Mighty.HID;
using monitor_customhmd.DriverComm;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using Valve.VR;
using static monitor_customhmd.UsbPacketDefs;

namespace monitor_customhmd
{
    public enum PacketDirection
    {
        Incoming,
        Outgoing
    }

    public partial class MonitorForm : Form
    {
        private NotifyIcon _trayIcon;
        //private CVRSystem _vrSystem;        
        private Thread _thread;
        private bool _running;

        private CommState State = CommState.Disconnected;

        private Dictionary<CommState, Icon> StateIcons;

        public static List<USBPacket> InPacketMonitor = new List<USBPacket>();
        public static List<USBPacket> OutPacketMonitor = new List<USBPacket>();
        public static readonly Queue<byte[]> OutgoingPackets = new Queue<byte[]>();

        public MonitorForm()
        {
            InitializeComponent();

            Icon = Properties.Resources.HeadSetActive;

            StateIcons = new Dictionary<CommState, Icon>()
            {
                { CommState.Disconnected, Properties.Resources.HeadSetWire},
                { CommState.Connected, Properties.Resources.HeadSetWhite},
                { CommState.Active, Properties.Resources.HeadSetActive}
            };

            IsDebug = mnuDebug.Checked = true;
            //WindowState = FormWindowState.Minimized;

            _trayIcon = new NotifyIcon();
            _trayIcon.Text = "Custom HMD Monitor";
            _trayIcon.DoubleClick += _trayIcon_DoubleClick;
            _trayIcon.ContextMenuStrip = trayMenu;
            SetTrayIcon();
            _trayIcon.Visible = true;
        }

        private void SetTrayIcon()
        {
            var ico = StateIcons[State];
            if (_trayIcon.Icon != ico)
                _trayIcon.Icon = ico;
        }

        private void _trayIcon_DoubleClick(object sender, EventArgs e)
        {
            Show();
            BringToFront();
            WindowState = FormWindowState.Normal;
            Activate();
        }
                
        private void MonitorForm_Load(object sender, EventArgs e)
        {
            IsVisible = true;
            _thread = new Thread(USBProcessor);
            _running = true;
            _thread.Start();
        }

        private void MonitorForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            _running = false;
            _thread.Join();
            _thread = null;
            _trayIcon.Dispose();
            _trayIcon = null;
            //if (ulMainHandle != 0)
            //    OpenVR.Overlay.DestroyOverlay(ulMainHandle);
            //OpenVR.Shutdown();
        }

        private void USBProcessor()
        {
            var _sharedMem = new ShMem();
            List<byte[]> outgoingPackets = null;
            HIDDev _usb = null;
            var data = new byte[33];
            DateTime lastOutgoing = DateTime.MinValue;
            while (_running)
            {
                if (_usb == null)
                {
                    var hd = HIDBrowse.Browse().FirstOrDefault(h => h.Vid == 0x1974 && h.Pid == 0x001);
                    if (hd != null)
                    {
                        _usb = new HIDDev();
                        _usb.Open(hd);
                        State = CommState.Connected;
                        _sharedMem.SetState(State);
                    }
                }
                try
                {
                    if (_usb != null)
                    {
                        outgoingPackets = _sharedMem.ReadOutgoingPackets();
                        if (outgoingPackets != null)
                            lock (OutgoingPackets)
                                foreach (var item in outgoingPackets)
                                    OutgoingPackets.Enqueue(item);
                        lock (OutgoingPackets)
                            if (OutgoingPackets.Count > 0 && (DateTime.Now - lastOutgoing).TotalMilliseconds >= 100)
                            {
                                lastOutgoing = DateTime.Now;
                                var d = OutgoingPackets.Dequeue(); //33 byte packets in queue
                                _usb.Write(d); //send before monitor process                                
                                if (IsDebug && IsVisible)
                                {
                                    var packet = StructFromBytes<USBPacket>(d, 1);
                                    packet.ParseFields();
                                    lock (OutPacketMonitor)
                                        OutPacketMonitor.Add(packet);
                                }
                            }

                        _usb.Read(data);
                        if (CheckPacketCrc(data, 1))
                        {
                            _sharedMem.WriteIncomingPacket(data); //send before monitor process
                            State = CommState.Active;
                            if (IsDebug && IsVisible)
                            {
                                //var x = Marshal.SizeOf(typeof(USBPacket));
                                var packet = StructFromBytes<USBPacket>(data, 1);
                                packet.ParseFields();
                                lock (InPacketMonitor)
                                    InPacketMonitor.Add(packet);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    if (_usb != null)
                    {
                        _usb.Close();
                        _usb.Dispose();
                    }
                    _usb = null;
                    State = CommState.Disconnected;
                    _sharedMem.SetState(CommState.Disconnected);

                    Thread.Sleep(100);
                }
            }
            if (_usb != null)
            {
                _usb.Close();
                _usb.Dispose();
            }
            _usb = null;
            State = CommState.Disconnected;
            _sharedMem.SetState(CommState.Disconnected);
            _sharedMem.Dispose();
        }

        private void tmrConsumer_Tick(object sender, EventArgs e)
        {
            SetTrayIcon();
            //bool done = false;
            //bool last = false;            

            IList<USBPacket> packets = null;
            lock (InPacketMonitor)
            {
                if (InPacketMonitor.Count > 0)
                {
                    packets = InPacketMonitor;
                    InPacketMonitor = new List<USBPacket>();
                }
            }

            if (packets != null)
            {                
                foreach (var p in packets)
                {
                    var packet = p;
                    OnPacket(PacketDirection.Incoming, ref packet);
                }
            }

            packets = null;
            lock (OutPacketMonitor)
            {
                if (OutPacketMonitor.Count > 0)
                {
                    packets = OutPacketMonitor;
                    OutPacketMonitor = new List<USBPacket>();
                }
            }

            if (packets != null)
            {                
                foreach (var p in packets)
                {
                    var packet = p;
                    OnPacket(PacketDirection.Outgoing, ref packet);
                }
            }

            /*
            if (dgData.SelectedRows != null && dgData.SelectedRows[0].Tag != null)
                pb.Invalidate();
            */
        }

        private readonly Dictionary<int, string> TabNames = new Dictionary<int, string>
        {
            { HMD_SOURCE, "Head Mounted Display"},
            { LEFTCTL_SOURCE, "Left Controller"},
            { RIGHTCTL_SOURCE, "Right Controller"},
            { BASESTATION_SOURCE, "Base Station"},
            { LIGHTHOUSE1_SOURCE, "Light House 1" },
            { LIGHTHOUSE2_SOURCE, "Light House 2" }
        };       

        private void OnPacket(PacketDirection direction, ref USBPacket packet)
        {
            var src = (byte)(packet.Header.Type & 0x0f);
            var pageName = TabNames[src];
            TabPage tab;

            if (!tabSources.TabPages.ContainsKey(pageName))
                tabSources.TabPages.Add(pageName, pageName);
            tab = tabSources.TabPages[pageName];
            var device = tab.Tag as TrackedDevice;
            if (device == null)
            {
                device = new TrackedDevice(src) { Dock = DockStyle.Fill };
                tab.Controls.Add(device);
                tab.Tag = device;                
            }
            device.ProcessPacket(direction, ref packet);
        } 
        
        private void MonitorForm_FormClosed(object sender, FormClosedEventArgs e)
        {
        }

        private void mnuExit_Click(object sender, EventArgs e)
        {
            Close();
        }

        private bool IsDebug { get; set; }

        private void mnuDebug_Click(object sender, EventArgs e)
        {
            mnuDebug.Checked = !mnuDebug.Checked;
            IsDebug = mnuDebug.Checked;
        }

        private bool IsVisible { get; set; }

        private void MonitorForm_Resize(object sender, EventArgs e)
        {
            if (WindowState == FormWindowState.Minimized)
            {
                IsVisible = false;
                Hide();
            }
            else
                IsVisible = true;
        }
    }
}

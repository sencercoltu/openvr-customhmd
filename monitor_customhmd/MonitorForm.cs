using monitor_customhmd.DriverComm;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using Valve.VR;
using static monitor_customhmd.UsbPacketDefs;
using System.Net.Sockets;
using System.IO;
using System.Drawing.Imaging;
using System.Net;
using LibUsbDotNet.WinUsb;
using LibUsbDotNet;
using LibUsbDotNet.Main;

namespace monitor_customhmd
{
    public enum PacketDirection
    {
        Incoming,
        Outgoing
    }

    public partial class MonitorForm : Form
    {
        //private int FPS;
        //private int _frameCount;
        //private SolidBrush _frameBrush = new SolidBrush(Color.Yellow);
        //private Font _frameFont = new Font(FontFamily.GenericSansSerif, 12);
        //private Pen _framePen = new Pen(Color.Yellow);

        private NotifyIcon _trayIcon;
        //private CVRSystem _vrSystem;        

        //private string DisplayAddress = "127.0.0.1";
        //private string DisplayAddress = "192.168.0.27";

        //private CommState _state = CommState.Uninitialized;

        private void SetState(CommState state, bool set)
        {
            var prevState = _sharedMem.State;
            var newState = _sharedMem.State;
            if (set) newState |= state; else newState &= ~state;
            if (prevState != newState)
                _sharedMem.State = newState;
        }

        private Dictionary<CommState, Icon> StateIcons { get; set; } = new Dictionary<CommState, Icon>()
            {
                { CommState.Uninitialized, Properties.Resources.HeadSetWire},
                { CommState.DriverActive, Properties.Resources.HeadSetWhite},
                { CommState.TrackerActive, Properties.Resources.HeadSetActiveNoDriver},
                { CommState.DriverActive | CommState.TrackerActive, Properties.Resources.HeadSetActive}
            };

        public static List<USBPacket> InPacketMonitor = new List<USBPacket>();
        public static List<USBPacket> OutPacketMonitor = new List<USBPacket>();
        public static readonly Queue<byte[]> OutgoingPackets = new Queue<byte[]>();

        private ShMem _sharedMem = null;

        //private object _socketLock = new object();
        //private TcpClient _tcpClient;
        //private Socket _socket;
        //private bool _isConnecting;
        //private PacketInfo _headerCache;
        //private IPAddress _serverAddr;
        //private IPEndPoint _endPoint;

        //private ScreenInfo _screenInfo;
        //private int _colorThreshold = 16;

        //private byte[] ScreenData = new byte[3840 * 2160 * 4];

        //private Image ScreenImage;

        public MonitorForm()
        {
            //JPEGFormat = GetEncoder(ImageFormat.Jpeg);
            //Encoder jpegEncoder = System.Drawing.Imaging.Encoder.Quality;
            //JPEGQuality = new EncoderParameters(1);

            //EncoderParameter qualityParameter = new EncoderParameter(jpegEncoder, 60L);
            //JPEGQuality.Param[0] = qualityParameter;

            InitializeComponent();

            Icon = Properties.Resources.HeadSetActive;

            IsDebug = mnuDebug.Checked = true;
            //WindowState = FormWindowState.Minimized;

            _trayIcon = new NotifyIcon();
            _trayIcon.Text = "Custom HMD Monitor";
            _trayIcon.DoubleClick += _trayIcon_DoubleClick;
            _trayIcon.ContextMenuStrip = trayMenu;
            _trayIcon.Visible = true;
        }

        private void SetStateIcon()
        {
            var ico = StateIcons[_sharedMem.State];
            if (_trayIcon.Icon != ico)
            {
                Icon = ico;
                _trayIcon.Icon = ico;
            }
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
            _sharedMem = new ShMem();
            _sharedMem.EnableWatchDog(false);
            IsVisible = true;
            tmrConsumer.Enabled = true;
        }

        //private ushort dispPacketCounter = 0;


        //private void RemoteDisplayProcessor()
        //{
        //    var receiveThread = new Thread(() =>
        //    {
        //        byte[] incoming = new byte[100 * 16];
        //        while (_running)
        //        {
        //            lock (_socketLock)
        //            {

        //                if (_tcpClient != null)
        //                {
        //                    try
        //                    {
        //                        var rotations = Math.Min(_tcpClient.Available / 16, 100);

        //                        if (rotations > 0)
        //                        {
        //                            if (State < CommState.ActiveNoDriver)
        //                                State = _sharedMem.IsDriverActive ? CommState.Active : CommState.ActiveNoDriver;

        //                            _tcpClient.Client.Receive(incoming, 0, 16 * rotations, SocketFlags.None);
        //                            var rotData = StructFromBytes<USBRotationData>(incoming, 16 * (rotations - 1));
        //                            //Debug.WriteLine("r:" + rotations + " w: " + rotData.w.ToString("F6") + " x: " + rotData.x.ToString("F6") + "y: " + rotData.y.ToString("F6") + "z:" + rotData.z.ToString("F6"));
        //                            var packet = USBPacket.Create((byte)(ROTATION_DATA | HMD_SOURCE), dispPacketCounter++, rotData);
        //                            var d = StructToBytes(packet);
        //                            SetPacketCrc(ref d);
        //                            _sharedMem.WriteIncomingPacket(d);
        //                        }
        //                    }
        //                    catch
        //                    {

        //                    }
        //                }
        //            }
        //        }

        //    });

        //    receiveThread.Start();

        //    while (_running)
        //    {
        //        var result = ProcessScreen();                
        //        if (!result)
        //            Thread.Sleep(10);
        //    }

        //    receiveThread.Join();
        //    receiveThread = null;

        //}

        //private bool ProcessScreen()
        //{
        //    var size = _sharedMem.GetScreenImage(out ScreenData);
        //    if (size <= 0) return false;

        //    _frameCount++;

        //    lock (_socketLock)
        //    {
        //        if (_tcpClient == null)
        //        {
        //            _isConnecting = false;
        //            _tcpClient = new TcpClient();
        //        }
        //    }            

        //    using (var ms = new MemoryStream(ScreenData))
        //    {
        //        ScreenImage = Image.FromStream(ms);
        //    }

        //    SendFrame(ScreenData, size);

        //    if (chkPreview.Checked)
        //        pbScreen.Invalidate();
        //    return true;
        //}

        //private void OnRemoteConnect(IAsyncResult ar)
        //{
        //    lock (_socketLock)
        //    {
        //        _isConnecting = false;
        //        try
        //        {
        //            _tcpClient.EndConnect(ar);
        //            _socket = _tcpClient.Client;
        //            State = _sharedMem.IsDriverActive ? CommState.Active : CommState.ActiveNoDriver;
        //            ResetRotation();
        //        }
        //        catch
        //        {
        //            _socket = null;
        //        }
        //    }
        //}

        private void MonitorForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            EnableOpenTrack(false);
            EnableCustomHMD(false);
            EnablePSVR(false);
            _trayIcon.Dispose();
            _trayIcon = null;

            tmrConsumer.Enabled = false;
            _sharedMem.EnableWatchDog(false);

            SetState(CommState.TrackerActive, false);

            _sharedMem.Dispose();
            _sharedMem = null;
            //if (ulMainHandle != 0)
            //    OpenVR.Overlay.DestroyOverlay(ulMainHandle);
            //OpenVR.Shutdown();
            //if (_socket != null && _socket.Connected)
            //{
            //    _socket.Shutdown(SocketShutdown.Both);
            //    _socket.Close();
            //}
            //lock (_socketLock)
            //{
            //    if (_tcpClient != null)
            //        _tcpClient.Dispose();
            //    _tcpClient = null;
            //}
        }

        //private const int LinesPerPacket = 2;

        private void tmrConsumer_Tick(object sender, EventArgs e)
        {
            SetStateIcon();
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
        }

        //private void GetBitmapDifference(byte[] diffMap, byte[] curBmp, byte[] prevBmp)
        //{
        //    var bpp = (int)(_screenInfo.Stride / _screenInfo.Width);
        //    byte pr, pg, pb, cr, cg, cb;
        //    //long rmean, r, g, b;
        //    //double d;
        //    for (var y = 0; y < _screenInfo.Height; y++)
        //    {
        //        for (var x = 0; x < _screenInfo.Width; x++)
        //        {
        //            var pos = (y * _screenInfo.Stride) + (x * bpp);

        //            pr = prevBmp[pos]; 
        //            pg = prevBmp[pos + 1]; 
        //            pb = prevBmp[pos + 2]; 

        //            cb = curBmp[pos]; 
        //            cg = curBmp[pos + 1]; 
        //            cr = curBmp[pos + 2];

        //            //if (pr == 0) pr = 1; if (pg == 0) pg = 1; if (pb == 0) pb = 1;
        //            //if (cr == 0) cr = 1; if (cg == 0) cg = 1; if (cb == 0) cb = 1;

        //            //rmean = ((long)cr + (long)pr) / 2;
        //            //r = (long)cr - (long)pr;
        //            //g = (long)cg - (long)pg;
        //            //b = (long)cb - (long)pb;

        //            //d = Math.Sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8));

        //            //var k = 1; // diffMap[pos + 3] / 32;
        //            //if (d > _colorThreshold * k)
        //            {
        //                diffMap[pos] = prevBmp[pos] = cr; pos++; // curBmp[pos++];
        //                diffMap[pos] = prevBmp[pos] = cg; pos++; // curBmp[pos++];
        //                diffMap[pos] = prevBmp[pos] = cb; pos++; // curBmp[pos++];
        //            }
        //            //else
        //            //{
        //            //    diffMap[pos++] = diffMap[pos++] = diffMap[pos++] = 0;
        //            //}
        //            //diffMap[pos] = 255;
        //        }
        //    }
        //}

        //private EncoderParameters JPEGQuality;

        //private ImageCodecInfo GetEncoder(ImageFormat format)
        //{
        //    ImageCodecInfo[] codecs = ImageCodecInfo.GetImageDecoders();
        //    foreach (ImageCodecInfo codec in codecs)
        //    {
        //        if (codec.FormatID == format.Guid)
        //        {
        //            return codec;
        //        }
        //    }
        //    return null;
        //}

        //private ImageCodecInfo JPEGFormat;

        //private byte[] SetBitmapBytes(PictureBox pbx, byte[] bytes)
        //{
        //    var bitmap = pbx.Tag as Bitmap;
        //    if (bitmap == null) return null;
        //    lock (pbx)
        //    {
        //        var bits = bitmap.LockBits(new Rectangle(0, 0, _screenInfo.Width, _screenInfo.Height), System.Drawing.Imaging.ImageLockMode.WriteOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
        //        Marshal.Copy(bytes, 0, bits.Scan0, Math.Min(bits.Stride * bits.Height, _screenInfo.Size()));
        //        bitmap.UnlockBits(bits);
        //    }                        
        //    pbx.Invalidate();
        //    lock (pbx)
        //    {
        //        using (var stream = new MemoryStream())
        //        {

        //            bitmap.Save(stream, JPEGFormat, JPEGQuality);
        //            return stream.ToArray();
        //        }
        //    }
        //}

        //private void SendFrame(byte[] bytes, int size)
        //{
        //    if (bytes == null) return;

        //    lock (_socketLock)
        //    {
        //        if (_socket == null)
        //        {
        //            if (!_isConnecting)
        //            {
        //                if (_tcpClient == null)
        //                    return;
        //                _isConnecting = true;
        //                _tcpClient.BeginConnect(DisplayAddress, 1974, OnRemoteConnect, null); //adb forward tcp:1974 tcp:1974                        
        //            }
        //            return;
        //        }
        //    }

        //    _headerCache.Size = size;
        //    var headerBytes = StructToBytes(_headerCache);
        //    if (!SendToRemoteDisplay(headerBytes)) return;
        //    if (!SendToRemoteDisplay(bytes)) return;


        //}

        //private bool SendToRemoteDisplay(byte[] data)
        //{
        //    lock (_socketLock)
        //    {
        //        if (_socket != null)
        //        {
        //            try
        //            {
        //                _socket.Send(data);
        //                return true;
        //            }
        //            catch
        //            {
        //                _tcpClient.Close();
        //                _tcpClient = null;
        //                _socket = null;
        //                return false;
        //            }
        //        }
        //        return false;
        //    }
        //}

        //private void OnSend(object state)
        //{
        //    //_udpPackets--;
        //}

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

        private void chkWatchDog_CheckedChanged(object sender, EventArgs e)
        {
            if (_sharedMem == null) return;
            _sharedMem.EnableWatchDog(chkWatchDog.Checked);
        }

        private void btnResetRotation_Click(object sender, EventArgs e)
        {
            ResetRotation();
        }

        private void ResetRotation()
        {
            var rotData = new USBRotationData();
            rotData.w = 1;
            rotData.x = 0;
            rotData.y = 0;
            rotData.z = 0;

            var packet = USBPacket.Create((byte)(ROTATION_DATA | HMD_SOURCE), (ushort)(DateTime.Now.Ticks / 1000), rotData);
            var d = StructToBytes(packet);
            SetPacketCrc(ref d);
            _sharedMem.WriteIncomingPacket(d);
        }

        private void pbScreen_Paint(object sender, PaintEventArgs e)
        {
            //if (ScreenImage == null) return;
            //if (!chkPreview.Checked) return;

            //e.Graphics.DrawImage(ScreenImage, 0, 0, pbScreen.Width, pbScreen.Height);
            //e.Graphics.DrawString("FPS: " + FPS, _frameFont, _frameBrush, 0, 0);
        }

        private void trackerUV_ValueChanged(object sender, EventArgs e)
        {

        }

        private static ushort LastDistSeq = 0;
        private void SendDistortion()
        {
            var distSeq = (ushort)(DateTime.Now.Ticks);
            if (distSeq == LastDistSeq) distSeq++;
            var distortionData = new USBDistortionData();
            distortionData.Reload = 1;
            LastDistSeq = distSeq;
            var command = USBCommandData.Create(CMD_DISTORTION, distortionData);
            var packet = USBPacket.Create((byte)(COMMAND_DATA | HMD_SOURCE), distSeq, command);
            var d = StructToBytes(packet);
            SetPacketCrc(ref d);
            _sharedMem.WriteIncomingPacket(d);
            Debug.WriteLine("Sent distortion");
        }

        private void btnResetDistortion_Click(object sender, EventArgs e)
        {
            SendDistortion();
        }

        private Thread OpenTrackThread = null;

        private void rdOpenTrack_CheckedChanged(object sender, EventArgs e)
        {
            EnablePSVR(rdPSVR.Checked);
            EnableCustomHMD(rdCustomHmd.Checked);
            EnableOpenTrack(rdOpenTrack.Checked);
            txtOpenTrackPort.Enabled = !rdOpenTrack.Checked;
        }

        private void EnableOpenTrack(bool en)
        {
            if (en && OpenTrackThread == null)
            {
                SetState(CommState.TrackerActive, true);
                var port = 0;
                if (!int.TryParse(txtOpenTrackPort.Text, out port) || port <= 0)
                {
                    MessageBox.Show(this, "Invalid UDP port", "OpenTrack Port Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    rdOpenTrack.Checked = false;
                    return;
                }

                OpenTrackThread = new Thread(() =>
                {
                    ushort sequence = 0;
                    var RemoteIpEndPoint = new IPEndPoint(IPAddress.Any, 0);
                    List<byte[]> outgoingPackets = null;
                    var lastOutgoing = DateTime.MinValue;
                    using (var uc = new UdpClient(port))
                    {
                        uc.Client.Blocking = false;
                        var sleep = 0;
                        while (OpenTrackThread != null)
                        {
                            SetState(CommState.DriverActive, _sharedMem.IsDriverActive);

                            sleep = 1;
                            if (uc.Client.Available >= OpenTrackPacket.Size)
                            {
                                var bytes = uc.Receive(ref RemoteIpEndPoint);
                                var packet = StructFromBytes<OpenTrackPacket>(bytes);

                                USBRotationData rotData;
                                USBPositionData posData;

                                packet.Init(out rotData, out posData);

                                var usbPacket = USBPacket.Create((byte)(ROTATION_DATA | HMD_SOURCE), sequence++, rotData);
                                var d = StructToBytes(usbPacket);
                                SetPacketCrc(ref d);
                                _sharedMem.WriteIncomingPacket(d);

                                if (IsDebug && IsVisible)
                                {
                                    lock (InPacketMonitor)
                                        InPacketMonitor.Add(usbPacket);
                                }


                                usbPacket = USBPacket.Create((byte)(POSITION_DATA | HMD_SOURCE), sequence++, posData);
                                d = StructToBytes(usbPacket);
                                SetPacketCrc(ref d);
                                _sharedMem.WriteIncomingPacket(d);

                                if (IsDebug && IsVisible)
                                {
                                    lock (InPacketMonitor)
                                        InPacketMonitor.Add(usbPacket);
                                }
                                sleep = 0;
                            }

                            //opentrack  doesnt receive data from us, just empty outgoing queue 
                            outgoingPackets = _sharedMem.ReadOutgoingPackets();

                            //this block is only for outputting data to monitor
                            if (outgoingPackets != null)
                            {
                                lock (OutgoingPackets)
                                {
                                    foreach (var item in outgoingPackets)
                                        OutgoingPackets.Enqueue(item);
                                }
                            }

                            lock (OutgoingPackets)
                            {
                                if (OutgoingPackets.Count > 0 && (DateTime.Now - lastOutgoing).TotalMilliseconds >= 100)
                                {
                                    lastOutgoing = DateTime.Now;
                                    var d = OutgoingPackets.Dequeue(); //33 byte packets in queue                                    
                                    if (IsDebug && IsVisible)
                                    {
                                        var packet = StructFromBytes<USBPacket>(d);
                                        packet.ParseFields();
                                        lock (OutPacketMonitor)
                                            OutPacketMonitor.Add(packet);
                                    }
                                }
                            }
                            Thread.Sleep(sleep);
                        }
                    }
                });
                OpenTrackThread.Start();
            }
            else if (!en && OpenTrackThread != null)
            {
                var t = OpenTrackThread;
                OpenTrackThread = null;
                t.Join();
                SetState(CommState.TrackerActive, false);
            }
        }

        private Thread CustomHMDThread = null;

        private bool OpenCustomDevice(out WinUsbDevice usb, out UsbEndpointReader reader, out UsbEndpointWriter writer)
        {
            usb = null;
            reader = null;
            writer = null;

            var device = UsbDevice.AllWinUsbDevices.FirstOrDefault(d => d.Vid == 0x1974 && d.Pid == 0x0001) as WinUsbRegistry;
            if (device != null)
            {
                if (device.Open(out usb))
                {
                    writer = usb.OpenEndpointWriter(WriteEndpointID.Ep01);
                    writer.Reset();
                    writer.Flush();

                    reader = usb.OpenEndpointReader(ReadEndpointID.Ep01);
                    reader.Reset();
                    reader.Flush();
                    reader.ReadBufferSize = 32;
                    reader.DataReceived += CustomDataReceived;
                    reader.DataReceivedEnabled = true;
                    return true;
                }
            }
            return false;
        }

        private void ProcessIncoming(byte[] data, USBPacket? packet = null)
        {
            if (data == null) return;
            if (CheckPacketCrc(data))
            {
                _sharedMem.WriteIncomingPacket(data); //send before monitor process                                    
                if (IsDebug && IsVisible)
                {
                    if (!packet.HasValue)
                    {
                        packet = StructFromBytes<USBPacket>(data);
                        packet.Value.ParseFields();
                    }
                    lock (InPacketMonitor)
                        InPacketMonitor.Add(packet.Value);
                }
            }
        }

        private void CustomDataReceived(object sender, EndpointDataEventArgs e)
        {
            var data = e.Buffer;
            ProcessIncoming(data, null);
        }

        private void CloseCustomDevice(ref WinUsbDevice usb, ref UsbEndpointReader reader, ref UsbEndpointWriter writer)
        {
            if (reader != null)
            {
                reader.DataReceivedEnabled = false;
                reader.DataReceived -= CustomDataReceived;
                reader.Dispose();
            }
            reader = null;
            if (writer != null)
                writer.Dispose();
            writer = null;
            if (usb != null)
                usb.Close();
            usb = null;
        }

        private void EnableCustomHMD(bool en)
        {
            if (en && CustomHMDThread == null)
            {
                CustomHMDThread = new Thread(() =>
                {
                    List<byte[]> outgoingPackets = null;
                    var lastOutgoing = DateTime.MinValue;

                    WinUsbDevice _usb = null;
                    UsbEndpointWriter _writer = null;
                    UsbEndpointReader _reader = null;

                    var data = new byte[32];
                    while (CustomHMDThread != null)
                    {
                        SetState(CommState.DriverActive, _sharedMem.IsDriverActive);

                        if (_usb == null)
                        {
                            OpenCustomDevice(out _usb, out _reader, out _writer);
                            if (_usb != null)
                            {
                                SetState(CommState.TrackerActive, true);
                            }
                            else
                            {
                                SetState(CommState.TrackerActive, false);
                                Thread.Sleep(10);
                            }
                        }
                        try
                        {
                            outgoingPackets = _sharedMem.ReadOutgoingPackets();
                            if (outgoingPackets != null)
                            {
                                lock (OutgoingPackets)
                                {
                                    foreach (var item in outgoingPackets)
                                        OutgoingPackets.Enqueue(item);
                                }
                            }

                            //empty output even if no usb connected
                            lock (OutgoingPackets)
                            {
                                if (OutgoingPackets.Count > 0 && (DateTime.Now - lastOutgoing).TotalMilliseconds >= 100)
                                {
                                    lastOutgoing = DateTime.Now;
                                    var d = OutgoingPackets.Dequeue(); //33 byte packets in queue                                    
                                    if (_writer != null)
                                    {
                                        int len;
                                        var err = _writer.Write(d, 100, out len); //send before monitor process                                
                                        if (err != ErrorCode.Ok)
                                            throw new Exception(UsbDevice.LastErrorString);
                                    }
                                    if (IsDebug && IsVisible)
                                    {
                                        var packet = StructFromBytes<USBPacket>(d);
                                        packet.ParseFields();
                                        lock (OutPacketMonitor)
                                            OutPacketMonitor.Add(packet);
                                    }
                                }
                            }
                        }
                        catch (Exception ex)
                        {
                            Debug.WriteLine(ex.Message);
                            CloseCustomDevice(ref _usb, ref _reader, ref _writer);
                            Thread.Sleep(100);
                        }
                    }

                    CloseCustomDevice(ref _usb, ref _reader, ref _writer);
                    SetState(CommState.TrackerActive, false);
                    UsbDevice.Exit();

                });
                CustomHMDThread.Start();
            }
            else if (!en && CustomHMDThread != null)
            {
                var t = CustomHMDThread;
                CustomHMDThread = null;
                t.Join();
                SetState(CommState.TrackerActive, false);
            }
        }

        private Thread PSVRThread = null;

        private void EnablePSVR(bool en)
        {
            if (en && PSVRThread == null)
            {
                PSVRThread = new Thread(() =>
                {
                    var psvr = new PSVRHandler();
                    psvr.OnPacketReceive = ProcessIncoming;
                    List<byte[]> outgoingPackets = null;
                    var lastOutgoing = DateTime.MinValue;

                    //also add move (bluetooth?)

                    while (PSVRThread != null)
                    {
                        SetState(CommState.DriverActive, _sharedMem.IsDriverActive);
                        SetState(CommState.TrackerActive, psvr.IsActive);

                        outgoingPackets = _sharedMem.ReadOutgoingPackets();
                        if (outgoingPackets != null)
                        {
                            lock (OutgoingPackets)
                            {
                                foreach (var item in outgoingPackets)
                                    OutgoingPackets.Enqueue(item);
                            }
                        }

                        //empty output even if no usb connected
                        lock (OutgoingPackets)
                        {
                            if (OutgoingPackets.Count > 0 && (DateTime.Now - lastOutgoing).TotalMilliseconds >= 100)
                            {
                                lastOutgoing = DateTime.Now;
                                var d = OutgoingPackets.Dequeue(); //33 byte packets in queue
                                                                   //interpret data (haptic feedback?)
                                var packet = StructFromBytes<USBPacket>(d);
                                packet.ParseFields();

                                psvr.ProcessOutgoing(packet);
                                if (IsDebug && IsVisible)
                                {
                                    lock (OutPacketMonitor)
                                        OutPacketMonitor.Add(packet);
                                }
                            }
                            else
                                Thread.Sleep(1);
                        }
                    }

                    psvr.Dispose();

                });
                PSVRThread.Start();
            }
            else if (!en && PSVRThread != null)
            {
                var t = PSVRThread;
                PSVRThread = null;
                t.Join();
                SetState(CommState.TrackerActive, false);
            }
        }

        //public static HmdQuaternion_t CreateFromYawPitchRoll(float yaw, float pitch, float roll)
        //{
        //    float rollOver2 = roll * 0.5f;
        //    float sinRollOver2 = (float)Math.Sin((double)rollOver2);
        //    float cosRollOver2 = (float)Math.Cos((double)rollOver2);
        //    float pitchOver2 = pitch * 0.5f;
        //    float sinPitchOver2 = (float)Math.Sin((double)pitchOver2);
        //    float cosPitchOver2 = (float)Math.Cos((double)pitchOver2);
        //    float yawOver2 = yaw * 0.5f;
        //    float sinYawOver2 = (float)Math.Sin((double)yawOver2);
        //    float cosYawOver2 = (float)Math.Cos((double)yawOver2);
        //    HmdQuaternion_t result;
        //    result.x = cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2;
        //    result.y = cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2;
        //    result.z = cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2;
        //    result.w = sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2;
        //    return result;
        //}
    }
}

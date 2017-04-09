using Mighty.HID;
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
        private Thread _commThread;
        private Thread _dispThread;
        private bool _running;

        private CommState State = CommState.Disconnected;

        private Dictionary<CommState, Icon> StateIcons;

        public static List<USBPacket> InPacketMonitor = new List<USBPacket>();
        public static List<USBPacket> OutPacketMonitor = new List<USBPacket>();
        public static readonly Queue<byte[]> OutgoingPackets = new Queue<byte[]>();

        private ShMem _sharedMem = null;

        private object _socketLock = new object();
        private TcpClient _tcpClient;
        private Socket _socket;
        private bool _isConnecting;
        private bool _isFirstConnect;
        private ScreenPartInfo _headerCache;
        //private IPAddress _serverAddr;
        //private IPEndPoint _endPoint;

        private ScreenInfo _screenInfo;
        private int _colorThreshold = 16;

        

        public MonitorForm()
        {
            JPEGFormat = GetEncoder(ImageFormat.Jpeg);
            Encoder jpegEncoder = System.Drawing.Imaging.Encoder.Quality;
            JPEGQuality = new EncoderParameters(1);

            EncoderParameter qualityParameter = new EncoderParameter(jpegEncoder, 50L);
            JPEGQuality.Param[0] = qualityParameter;

            InitializeComponent();

            Icon = Properties.Resources.HeadSetActive;

            StateIcons = new Dictionary<CommState, Icon>()
            {
                { CommState.Disconnected, Properties.Resources.HeadSetWire},
                { CommState.Connected, Properties.Resources.HeadSetWhite},
                { CommState.ActiveNoDriver, Properties.Resources.HeadSetActiveNoDriver},
                { CommState.Active, Properties.Resources.HeadSetActive}
            };

            IsDebug = mnuDebug.Checked = true;
            //WindowState = FormWindowState.Minimized;

            _trayIcon = new NotifyIcon();
            _trayIcon.Text = "Custom HMD Monitor";
            _trayIcon.DoubleClick += _trayIcon_DoubleClick;
            _trayIcon.ContextMenuStrip = trayMenu;
            SetStateIcon();
            _trayIcon.Visible = true;
            
        }

        private void SetStateIcon()
        {
            var ico = StateIcons[State];
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
            //_serverAddr = IPAddress.Parse("192.168.0.27");
            //_endPoint = new IPEndPoint(_serverAddr, 2222);

            

            _sharedMem = new ShMem();
            _sharedMem.EnableWatchDog(false);
            IsVisible = true;
            _commThread = new Thread(USBProcessor);
            _dispThread = new Thread(DisplayProcessor);
            _running = true;
            _commThread.Start();
            _dispThread.Start();
        }

        private ushort dispPacketCounter = 0;

        private void DisplayProcessor()
        {
            var receiveThread = new Thread(() => 
            {
                byte[] incoming = new byte[100 * 16];

                while (_running)
                {
                    lock (_socketLock)
                    {
                        if (_tcpClient != null)
                        {
                            var rotations = Math.Min(_tcpClient.Available / 16, 100);

                            if (rotations > 0)
                            {
                                if (State < CommState.ActiveNoDriver)
                                    State = _sharedMem.IsDriverActive ? CommState.Active : CommState.ActiveNoDriver;

                                _tcpClient.Client.Receive(incoming, 0, 16 * rotations, SocketFlags.None);
                                var rotData = StructFromBytes<USBRotationData>(incoming, 16 * (rotations - 1));
                                //Debug.WriteLine("r:" + rotations + " w: " + rotData.w.ToString("F6") + " x: " + rotData.x.ToString("F6") + "y: " + rotData.y.ToString("F6") + "z:" + rotData.z.ToString("F6"));
                                var packet = USBPacket.Create((byte)(ROTATION_DATA | HMD_SOURCE), dispPacketCounter++, rotData);
                                var d = StructToBytes(packet);
                                SetPacketCrc(ref d);
                                _sharedMem.WriteIncomingPacket(d);
                            }
                        }
                    }
                }
            });

            receiveThread.Start();

            while (_running)
            {
                var updates = _sharedMem.GetScreenInfo(ref _screenInfo);
                //Debug.WriteLine("Updates: " + updates);
                if (updates > 0)
                {
                    lock (_socketLock)
                    {
                        if (_tcpClient == null)
                        {
                            _isConnecting = false;
                            _tcpClient = new TcpClient();
                        }
                    }

                    var size = _screenInfo.Size();
                    if (BitmapSize != size)
                    {
                        BitmapSize = size;
                        LeftDiffBitmap = new byte[BitmapSize];
                        RightDiffBitmap = new byte[BitmapSize];

                        LeftBitmap = new byte[BitmapSize];
                        RemoteLeftBitmap = new byte[BitmapSize];

                        //pbLeft.Width = _screenInfo.Width;
                        //pbLeft.Height = _screenInfo.Height;
                        pbLeft.Tag = new Bitmap(_screenInfo.Width, _screenInfo.Height, System.Drawing.Imaging.PixelFormat.Format32bppRgb);

                        RightBitmap = new byte[BitmapSize];
                        RemoteRightBitmap = new byte[BitmapSize];

                        //pbRight.Left = pbLeft.Left + pbLeft.Width;
                        //pbRight.Width = _screenInfo.Width;
                        //pbRight.Height = _screenInfo.Height;
                        pbRight.Tag = new Bitmap(_screenInfo.Width, _screenInfo.Height, System.Drawing.Imaging.PixelFormat.Format32bppRgb);
                    }
                    _sharedMem.GetScreenBitmap(ref _screenInfo, out LeftBitmap, out RightBitmap);

                    GetBitmapDifference(LeftDiffBitmap, LeftBitmap, RemoteLeftBitmap);
                    GetBitmapDifference(RightDiffBitmap, RightBitmap, RemoteRightBitmap);

                    var leftJpegBytes = SetBitmapBytes(pbLeft, RemoteLeftBitmap);
                    var rightJPegBytes = SetBitmapBytes(pbRight, RemoteRightBitmap);

                    //cnt++;
                    //File.WriteAllBytes("D:\\x\\L" + cnt + ".jpg", leftBytes);
                    //File.WriteAllBytes("D:\\x\\R" + cnt + ".jpg", rightBytes);


                    SendFrame(EVREye.Eye_Left, leftJpegBytes, ref _screenInfo);
                    SendFrame(EVREye.Eye_Right, rightJPegBytes, ref _screenInfo);
                }
                else
                    Thread.Sleep(10);
            }

            receiveThread.Join();
            receiveThread = null;

        }

        private void OnRemoteConnect(IAsyncResult ar)
        {
            lock (_socketLock)
            {
                _isConnecting = false;
                try
                {
                    _tcpClient.EndConnect(ar);
                    BitmapSize = 0;
                    _socket = _tcpClient.Client;
                    _isFirstConnect = true;
                    State = _sharedMem.IsDriverActive ? CommState.Active : CommState.ActiveNoDriver;
                }
                catch
                {
                    _socket = null;
                }
            }
        }

        private void MonitorForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            _running = false;
            _commThread.Join();
            _commThread = null;
            _dispThread.Join();
            _dispThread = null;
            _trayIcon.Dispose();
            _trayIcon = null;
            _sharedMem.Dispose();
            _sharedMem = null;
            //if (ulMainHandle != 0)
            //    OpenVR.Overlay.DestroyOverlay(ulMainHandle);
            //OpenVR.Shutdown();
            if (_socket != null && _socket.Connected)
            {
                _socket.Shutdown(SocketShutdown.Both);
                _socket.Close();
            }
            lock (_socketLock)
            {
                if (_tcpClient != null)
                    _tcpClient.Dispose();
                _tcpClient = null;
            }
        }

        private void USBProcessor()
        {
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
                            State = _sharedMem.IsDriverActive ? CommState.Active : CommState.ActiveNoDriver;
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
                    _sharedMem.SetState(State);

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
        }

        private int BitmapSize;

        private byte[] LeftBitmap = null;
        private byte[] RightBitmap = null;

        private byte[] RemoteLeftBitmap = null;
        private byte[] RemoteRightBitmap = null;

        private byte[] LeftDiffBitmap = null;
        private byte[] RightDiffBitmap = null;

        private const int LinesPerPacket = 2;

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
        
        private void GetBitmapDifference(byte[] diffMap, byte[] curBmp, byte[] prevBmp)
        {
            var bpp = (int)(_screenInfo.Stride / _screenInfo.Width);
            byte pr, pg, pb, cr, cg, cb;
            //long rmean, r, g, b;
            //double d;
            for (var y = 0; y < _screenInfo.Height; y++)
            {
                for (var x = 0; x < _screenInfo.Width; x++)
                {
                    var pos = (y * _screenInfo.Stride) + (x * bpp);

                    pr = prevBmp[pos]; 
                    pg = prevBmp[pos + 1]; 
                    pb = prevBmp[pos + 2]; 

                    cb = curBmp[pos]; 
                    cg = curBmp[pos + 1]; 
                    cr = curBmp[pos + 2];

                    //if (pr == 0) pr = 1; if (pg == 0) pg = 1; if (pb == 0) pb = 1;
                    //if (cr == 0) cr = 1; if (cg == 0) cg = 1; if (cb == 0) cb = 1;

                    //rmean = ((long)cr + (long)pr) / 2;
                    //r = (long)cr - (long)pr;
                    //g = (long)cg - (long)pg;
                    //b = (long)cb - (long)pb;

                    //d = Math.Sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8));

                    //var k = 1; // diffMap[pos + 3] / 32;
                    //if (d > _colorThreshold * k)
                    {
                        diffMap[pos] = prevBmp[pos] = cr; pos++; // curBmp[pos++];
                        diffMap[pos] = prevBmp[pos] = cg; pos++; // curBmp[pos++];
                        diffMap[pos] = prevBmp[pos] = cb; pos++; // curBmp[pos++];
                    }
                    //else
                    //{
                    //    diffMap[pos++] = diffMap[pos++] = diffMap[pos++] = 0;
                    //}
                    //diffMap[pos] = 255;
                }
            }
        }

        private EncoderParameters JPEGQuality;
        
        private ImageCodecInfo GetEncoder(ImageFormat format)
        {
            ImageCodecInfo[] codecs = ImageCodecInfo.GetImageDecoders();
            foreach (ImageCodecInfo codec in codecs)
            {
                if (codec.FormatID == format.Guid)
                {
                    return codec;
                }
            }
            return null;
        }

        private ImageCodecInfo JPEGFormat;

        private byte[] SetBitmapBytes(PictureBox pbx, byte[] bytes)
        {
            var bitmap = pbx.Tag as Bitmap;
            if (bitmap == null) return null;
            lock (pbx)
            {
                var bits = bitmap.LockBits(new Rectangle(0, 0, _screenInfo.Width, _screenInfo.Height), System.Drawing.Imaging.ImageLockMode.WriteOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                Marshal.Copy(bytes, 0, bits.Scan0, Math.Min(bits.Stride * bits.Height, _screenInfo.Size()));
                bitmap.UnlockBits(bits);
            }                        
            pbx.Invalidate();
            lock (pbx)
            {
                using (var stream = new MemoryStream())
                {
                    
                    bitmap.Save(stream, JPEGFormat, JPEGQuality);
                    return stream.ToArray();
                }
            }
        }

        private void SendFrame(EVREye eye, byte[] bytes, ref ScreenInfo info)
        {
            if (bytes == null) return;

            lock (_socketLock)
            {
                if (_socket == null)
                {
                    if (!_isConnecting)
                    {
                        if (_tcpClient == null)
                            return;
                        _isConnecting = true;
                        _tcpClient.BeginConnect("127.0.0.1", 1974, OnRemoteConnect, null); //adb forward tcp:1974 tcp:1974                        
                    }
                    return;
                }
            }

            if (_isFirstConnect)
            {
                var infoBytes = StructToBytes(_screenInfo);
                _isFirstConnect = false;
                if (!SendToDisplay(infoBytes)) return;
            }

            _headerCache.Eye = (int)eye;
            _headerCache.Size = bytes.Length;                
            var headerBytes = StructToBytes(_headerCache);
            if (!SendToDisplay(headerBytes)) return;
            if (!SendToDisplay(bytes)) return;


        }

        private bool SendToDisplay(byte[] data)
        {
            lock (_socketLock)
            {
                if (_socket != null)
                {
                    try
                    {
                        _socket.Send(data);
                        return true;
                    }
                    catch
                    {
                        _tcpClient.Close();
                        _tcpClient = null;
                        _socket = null;
                        return false;
                    }
                }
                return false;
            }
        }

        private void OnSend(object state)
        {
            //_udpPackets--;
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

        private void chkWatchDog_CheckedChanged(object sender, EventArgs e)
        {
            if (_sharedMem == null) return;
            _sharedMem.EnableWatchDog(chkWatchDog.Checked);
        }


        private void eyePaint(object sender, PaintEventArgs e)
        {
            var pbx = (sender as PictureBox);
            var img = pbx.Tag as Bitmap; 
            if (img == null) return;
            lock(pbx)
                e.Graphics.DrawImage(img, 0, 0, pbx.Width, pbx.Height);
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

        public static HmdQuaternion_t CreateFromYawPitchRoll(float yaw, float pitch, float roll)
        {
            float rollOver2 = roll * 0.5f;
            float sinRollOver2 = (float)Math.Sin((double)rollOver2);
            float cosRollOver2 = (float)Math.Cos((double)rollOver2);
            float pitchOver2 = pitch * 0.5f;
            float sinPitchOver2 = (float)Math.Sin((double)pitchOver2);
            float cosPitchOver2 = (float)Math.Cos((double)pitchOver2);
            float yawOver2 = yaw * 0.5f;
            float sinYawOver2 = (float)Math.Sin((double)yawOver2);
            float cosYawOver2 = (float)Math.Cos((double)yawOver2);
            HmdQuaternion_t result;
            result.x = cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2;
            result.y = cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2;
            result.z = cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2;
            result.w = sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2;
            return result;
        }
    }
}

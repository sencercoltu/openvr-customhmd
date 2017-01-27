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
        private ContextMenu _trayMenu;
        //private CVRSystem _vrSystem;        
        private Thread _thread;
        private bool _running;

        private CommState State = CommState.Disconnected;

        private Dictionary<CommState, Icon> StateIcons;

        public static List<USBPacket> InPacketMonitor = new List<USBPacket>();
        public static List<USBPacket> OutPacketMonitor = new List<USBPacket>();
        public static readonly Queue<byte[]> OutgoingPackets = new Queue<byte[]>();

        //private DataGridViewRow InfoRow;
        //private DataGridViewRow AccelRow;
        //private DataGridViewRow AccelOffsetRow;
        //private DataGridViewRow GyroRow;
        //private DataGridViewRow GyroOffsetRow;
        //private DataGridViewRow MagRow;
        //private DataGridViewRow MagOffsetRow;
        //private DataGridViewRow RotationRow;
        //private DataGridViewRow PositionRow;

        Camera cam = new Camera();
        Axes axis = null;
        Gravity grav = null;

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


            Visible = false;
            _trayMenu = new ContextMenu();
            _trayMenu.MenuItems.Add("Exit", ExitApp);

            _trayIcon = new NotifyIcon();
            _trayIcon.Text = "Custom HMD Monitor";
            _trayIcon.DoubleClick += _trayIcon_DoubleClick;
            _trayIcon.ContextMenu = _trayMenu;
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
            Focus();
        }

        private void ExitApp(object sender, EventArgs e)
        {
            Close();
        }

        private void MonitorForm_Load(object sender, EventArgs e)
        {
            //var err = EVRInitError.Driver_Failed;
            //_vrSystem = OpenVR.Init(ref err, EVRApplicationType.VRApplication_Overlay);
            //OpenVR.GetGenericInterface(OpenVR.IVRCompositor_Version, ref err);
            //OpenVR.GetGenericInterface(OpenVR.IVROverlay_Version, ref err);            
            axis = new Axes(100);
            axis.Center = new Point3d((pb.Width) / 2, (pb.Height) / 2, 0);
            cam.Location = new Point3d(axis.Center.X, axis.Center.Y, -500);

            grav = new Gravity(100);
            grav.Center = new Point3d((pb.Width) / 2, (pb.Height) / 2, 0);

            //testSensor.MaxPoints = pb.Width - 10;

            /*
            InfoRow = dgData.Rows[dgData.Rows.Add()]; InfoRow.Cells[0].Value = "Info";
            RotationRow = dgData.Rows[dgData.Rows.Add()]; RotationRow.Cells[0].Value = "Rotation"; RotationRow.Tag = axis;
            PositionRow = dgData.Rows[dgData.Rows.Add()]; PositionRow.Cells[0].Value = "Position"; PositionRow.Tag = RotationRow.Tag;
            AccelRow = dgData.Rows[dgData.Rows.Add()]; AccelRow.Cells[0].Value = "Accel/Gravity"; //AccelRow.Tag = new KalmanGroup(pb.Width - 10, 2, 2, 0, 0f);
            AccelOffsetRow = dgData.Rows[dgData.Rows.Add()]; AccelOffsetRow.Cells[0].Value = "Averages"; AccelOffsetRow.Tag = new float[3];
            GyroRow = dgData.Rows[dgData.Rows.Add()]; GyroRow.Cells[0].Value = "Gyro/CompAccel"; //GyroRow.Tag = new KalmanGroup(pb.Width - 10, 2, 2, 0, 0f);
            GyroOffsetRow = dgData.Rows[dgData.Rows.Add()]; GyroOffsetRow.Cells[0].Value = "Averages"; GyroOffsetRow.Tag = new float[3];
            MagRow = dgData.Rows[dgData.Rows.Add()]; MagRow.Cells[0].Value = "Mag/Velocity"; //MagRow.Tag = new KalmanGroup(pb.Width - 10, 2, 2, 0, 0f);
            MagOffsetRow = dgData.Rows[dgData.Rows.Add()]; MagOffsetRow.Cells[0].Value = "Averages"; MagOffsetRow.Tag = new float[3];
            */
            //ResetOffsets();

            foreach (var item in Enum.GetValues(typeof(RawModes)))
            {
                comboBox1.Items.Add(item);
            }


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
                                var packet = StructFromBytes<USBPacket>(d, 1);
                                packet.ParseFields();
                                lock (OutPacketMonitor)
                                    OutPacketMonitor.Add(packet);
                            }

                        _usb.Read(data);
                        if (CheckPacketCrc(data, 1))
                        {
                            _sharedMem.WriteIncomingPacket(data); //send before monitor process
                            //var x = Marshal.SizeOf(typeof(USBPacket));
                            var packet = StructFromBytes<USBPacket>(data, 1);
                            packet.ParseFields();
                            lock (InPacketMonitor)
                                InPacketMonitor.Add(packet);
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
                State = CommState.Active;
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
                State = CommState.Active;
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

        //private ushort[] lastSequences = new ushort[MAX_SOURCE];
        //private int seqErrors = 0;

        //SensorTest testSensor = new SensorTest();

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

        //private void OnIncomingPacket(ref USBPacket packet)
        //{
        //    var src = packet.Header.Type & 0x0f;
        //    var pageName = "#" + TabNames[src];
        //    TabPage tab;

        //    if (!tabSources.TabPages.ContainsKey(pageName))
        //    {
        //        tab = new TabPage(pageName);
        //        tabSources.TabPages.Add(tab);
        //    }
        //    else
        //        tab = tabSources.TabPages[pageName];



        //    var type = packet.Header.Type & 0xf0;

        //    var lastSequence = lastSequences[src];

        //    if (packet.Header.Sequence != lastSequence + 1 && lastSequence > 0)
        //    {
        //        seqErrors += (packet.Header.Sequence - lastSequence);
        //        //if (last)
        //        //{
        //            //InfoRow.Cells[1].Value = "Missed: " + (packet.Header.Sequence - lastSequence);
        //            //InfoRow.Cells[2].Value = "Total: " + seqErrors;
        //            //InfoRow.Cells[3].Value = "Last: " + packet.Header.Sequence;
        //            //InfoRow.Cells[4].Value = "Current: " + lastSequence;
        //        //}
        //    }
        //    lastSequences[src] = packet.Header.Sequence;

            
        //    switch (type)
        //    {
        //        case POSITION_DATA:
        //            //if (last)
        //            {
        //                //PositionRow.Cells[1].Value = packet.Header.Sequence;
        //                //PositionRow.Cells[3].Value = packet.Position.Position[0].ToString("N3");
        //                //PositionRow.Cells[4].Value = packet.Position.Position[1].ToString("N3");
        //                //PositionRow.Cells[5].Value = packet.Position.Position[2].ToString("N3");
        //            }
        //            break;
        //        case ROTATION_DATA:
        //            //if (last)
        //            {
        //                //RotationRow.Cells[1].Value = packet.Header.Sequence;
        //                //RotationRow.Cells[2].Value = packet.Rotation.w.ToString("N6");
        //                //RotationRow.Cells[3].Value = packet.Rotation.x.ToString("N6");
        //                //RotationRow.Cells[4].Value = packet.Rotation.y.ToString("N6");
        //                //RotationRow.Cells[5].Value = packet.Rotation.z.ToString("N6");
        //                ////}
        //                ////{
        //                var q = new Quaternion(packet.Rotation.w, packet.Rotation.x, packet.Rotation.y, packet.Rotation.z);
        //                //var axis = RotationRow.Tag as Axes;
        //                //axis.RotateAt(new Point3d(0, 0, 0), Quaternion.Identity);
        //                axis.RotateAt(axis.Center, q);
        //            }
        //            break;
        //        case TRIGGER_DATA:
        //            break;
        //        case COMMAND_DATA:
        //            if (packet.Command.Command == CMD_RAW_DATA)
        //            {
        //                /*
        //                //KalmanGroup group;
        //                //if (last)
        //                {
        //                    AccelRow.Cells[1].Value = packet.Header.Sequence;
        //                    AccelRow.Cells[3].Value = packet.Command.Raw.Accel[0];
        //                    AccelRow.Cells[4].Value = packet.Command.Raw.Accel[1];
        //                    AccelRow.Cells[5].Value = packet.Command.Raw.Accel[2];

        //                    //testSensor.Raw[0] = packet.Command.Raw.Accel[0];
        //                    //testSensor.Raw[1] = packet.Command.Raw.Accel[1];
        //                    //testSensor.Raw[2] = packet.Command.Raw.Accel[2];
        //                    //testSensor.ProcessNew();
        //                }
        //                //group = AccelRow.Tag as KalmanGroup;
        //                //group.Process(new float[] { ToGravity(packet.Command.Raw.Accel[0]), ToGravity(packet.Command.Raw.Accel[1]), ToGravity(packet.Command.Raw.Accel[2]) });

        //                //if (last)
        //                {
        //                    GyroRow.Cells[1].Value = packet.Header.Sequence;
        //                    GyroRow.Cells[3].Value = packet.Command.Raw.Gyro[0];
        //                    GyroRow.Cells[4].Value = packet.Command.Raw.Gyro[1];
        //                    GyroRow.Cells[5].Value = packet.Command.Raw.Gyro[2];
        //                }
        //                //group = GyroRow.Tag as KalmanGroup;
        //                /group.Process(new float[] { ToDegree(packet.Command.Raw.Gyro[0]), ToDegree(packet.Command.Raw.Gyro[1]), ToDegree(packet.Command.Raw.Gyro[2]) });

        //                //if (last)
        //                {
        //                    MagRow.Cells[1].Value = packet.Header.Sequence;
        //                    MagRow.Cells[3].Value = packet.Command.Raw.Mag[0];
        //                    MagRow.Cells[4].Value = packet.Command.Raw.Mag[1];
        //                    MagRow.Cells[5].Value = packet.Command.Raw.Mag[2];
        //                }
        //                group = MagRow.Tag as KalmanGroup;
        //                group.Process(new float[] { ToGauss(packet.Command.Raw.Mag[0]), ToGauss(packet.Command.Raw.Mag[1]), ToGauss(packet.Command.Raw.Mag[2]) });

        //                if ((packet.Command.Raw.State == (byte)DebugModes.Raw) ||
        //                    (packet.Command.Raw.State == (byte)DebugModes.Filtered) ||
        //                    (packet.Command.Raw.State == (byte)DebugModes.Compensated))
        //                {
        //                    var offsets = AccelOffsetRow.Tag as float[];
        //                    for (int i = 0; i < 3; i++)
        //                    {
        //                        offsets[i] = (offsets[i] * 0.99f) + (0.01f * packet.Command.Raw.Accel[i]);
        //                        //if (last)
        //                            AccelOffsetRow.Cells[3 + i].Value = (int)offsets[i];
        //                    }


        //                    offsets = GyroOffsetRow.Tag as float[];
        //                    for (int i = 0; i < 3; i++)
        //                    {
        //                        offsets[i] = (offsets[i] * 0.99f) + (0.01f * packet.Command.Raw.Gyro[i]);
        //                        //if (last)
        //                            GyroOffsetRow.Cells[3 + i].Value = (int)offsets[i];
        //                    }

        //                    offsets = MagOffsetRow.Tag as float[];
        //                    for (int i = 0; i < 3; i++)
        //                    {
        //                        offsets[i] = (offsets[i] * 0.99f) + (0.01f * packet.Command.Raw.Mag[i]);
        //                        //if (last)
        //                            MagOffsetRow.Cells[3 + i].Value = (int)offsets[i];
        //                    }
        //                }
        //                else if (packet.Command.Raw.State == (byte)DebugModes.Vectors)
        //                {
        //                    var q = new Quaternion(1, packet.Command.Raw.Accel[0] / 1000.0f, packet.Command.Raw.Accel[1] / 1000.0f, packet.Command.Raw.Accel[2] / 1000.0f);
        //                    grav.RotateAt(grav.Center, q);
        //                }
        //                */
        //            }
        //            break;
        //    }


        //    //lblSequence.Text = packet.Header.Sequence.ToString();
        //}

        //private float ToGravity(short val)
        //{
        //    var f = (float)val;
        //    f *= pb.Height / (2048.0f * 4 * ((float)(axisscale.Value) / 1000.0f));
        //    return f;
        //}

        //private float ToDegree(short val)
        //{
        //    var f = (float)val;
        //    f *= (pb.Height * 249.75f) / (32768.0f * 250.0f * 3.0f * ((float)(axisscale.Value) / 1000.0f));
        //    return f;
        //}

        //private float ToGauss(short val)
        //{
        //    var f = (float)val;
        //    f *= (pb.Height) / (1.6f * 1200.0f * 4 * ((float)(axisscale.Value) / 1000.0f));
        //    return f;
        //}

        //private void ResetOffsets()
        //{
        //    float[] offsets;
        //    offsets = AccelOffsetRow.Tag as float[];
        //    for (var i = 0; i < 3; i++)
        //        offsets[i] = 0;

        //    offsets = GyroOffsetRow.Tag as float[];
        //    for (var i = 0; i < 3; i++)
        //        offsets[i] = 0;

        //    offsets = MagOffsetRow.Tag as float[];
        //    for (var i = 0; i < 3; i++)
        //        offsets[i] = 0;
        //}


        private void button2_Click(object sender, EventArgs e)
        {
            //ResetOffsets();
            if (comboBox1.SelectedIndex == -1) return;

            var rawData = new USBRawData();
            rawData.State = (byte)comboBox1.SelectedIndex;
            var command = USBCommandData.Create(CMD_RAW_DATA, rawData);
            var packet = USBPacket.Create(COMMAND_DATA | HMD_SOURCE, (ushort)(DateTime.Now.Ticks / 1000), command);
            var d = StructToBytes(packet);
            SetPacketCrc(ref d);
            lock (OutgoingPackets)
                OutgoingPackets.Enqueue(d);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            //seqErrors = 0;
            //lastSequence = 0;

            //InfoRow.Cells[1].Value = "Missed: 0";
            //InfoRow.Cells[2].Value = "Total: 0";
            //InfoRow.Cells[3].Value = "Last: 0";
            //InfoRow.Cells[4].Value = "Current: 0";


            //var calibData = new USBCalibrationData();

            //if (AccelOffsetRow.Selected)
            //    calibData.Sensor = SENSOR_ACCEL;
            //else if (GyroOffsetRow.Selected)
            //    calibData.Sensor = SENSOR_GYRO;
            //else if (MagOffsetRow.Selected)
            //    calibData.Sensor = SENSOR_MAG;
            //else
            //    return;

            //var posScales = new short[3];
            //short.TryParse(txtPosX.Text, out posScales[0]);
            //short.TryParse(txtPosY.Text, out posScales[1]);
            //short.TryParse(txtPosZ.Text, out posScales[2]);

            //var negScales = new short[3];
            //short.TryParse(txtNegX.Text, out negScales[0]);
            //short.TryParse(txtNegY.Text, out negScales[1]);
            //short.TryParse(txtNegZ.Text, out negScales[2]);


            //calibData.PosScale = posScales;
            //calibData.NegScale = negScales;

            //var command = USBCommandData.Create(CMD_CALIBRATE, calibData);
            //var packet = USBPacket.Create(COMMAND_DATA | HMD_SOURCE, (ushort)(DateTime.Now.Ticks / 1000), command);
            //var d = StructToBytes(packet);
            //SetPacketCrc(ref d);
            //lock (_outgoingPackets)
            //    _outgoingPackets.Enqueue(d);


            //calibData.Sensor = SENSOR_NONE;
            //command = USBCommandData.Create(CMD_CALIBRATE, calibData);
            //packet = USBPacket.Create(COMMAND_DATA | HMD_SOURCE, (ushort)((DateTime.Now.Ticks / 1000) + 1), command);

            //d = StructToBytes(packet);
            //SetPacketCrc(ref d);
            //lock (_outgoingPackets)
            //    _outgoingPackets.Enqueue(d);

        }

        Pen axisPen = new Pen(Color.LightGray, 2);
        Pen xPen = new Pen(Color.Red, 1);
        Pen yPen = new Pen(Color.Green, 1);
        Pen zPen = new Pen(Color.Blue, 1);

        private void pb_Paint(object sender, PaintEventArgs e)
        {
            /*
            if (false)
            {
                var h = pb.Height / 2;
                e.Graphics.Clear(Color.Black);
                e.Graphics.DrawLine(axisPen, 0, h, pb.Width, h);
                var rawList = testSensor.PointsRaw as List<float[]>;
                if (rawList.Count > 1)
                {
                    float[] f = rawList[0];
                    var ay = (int)(h - f[0]);
                    var gy = (int)(h - f[1]);
                    var my = (int)(h - f[2]);
                    for (int i = 1; i < rawList.Count; i++)
                    {
                        f = rawList[i];
                        e.Graphics.DrawLine(xPen, i, ay, i + 1, h - f[0]); ay = (int)(h - f[0]);
                        e.Graphics.DrawLine(yPen, i, gy, i + 1, h - f[1]); gy = (int)(h - f[1]);
                        e.Graphics.DrawLine(zPen, i, my, i + 1, h - f[2]); my = (int)(h - f[2]);
                    }
                }

                var filteredList = testSensor.PointsFiltered as List<float[]>;
                if (filteredList.Count > 1)
                {
                    float[] f = filteredList[0];
                    var ay = (int)(h - f[0]);
                    var gy = (int)(h - f[1]);
                    var my = (int)(h - f[2]);
                    for (int i = 1; i < filteredList.Count; i++)
                    {
                        f = filteredList[i];
                        e.Graphics.DrawLine(yPen, i, ay, i + 1, h - f[0]); ay = (int)(h - f[0]);
                        e.Graphics.DrawLine(yPen, i, gy, i + 1, h - f[1]); gy = (int)(h - f[1]);
                        e.Graphics.DrawLine(zPen, i, my, i + 1, h - f[2]); my = (int)(h - f[2]);
                    }
                }

                return;
            }
            
            if (dgData.SelectedRows == null || dgData.SelectedRows[0].Tag == null) return;
            var group = dgData.SelectedRows[0].Tag as KalmanGroup;
            if (group != null)
            {
                var list = group.Points as List<float[]>;
                if (list.Count > 1)
                {
                    var h = pb.Height / 2;
                    e.Graphics.Clear(Color.Black);
                    e.Graphics.DrawLine(axisPen, 0, h, pb.Width, h);
                    float[] f = list[0];
                    var ay = (int)(h - f[0]);
                    var gy = (int)(h - f[1]);
                    var my = (int)(h - f[2]);
                    for (int i = 1; i < list.Count; i++)
                    {
                        f = list[i];
                        e.Graphics.DrawLine(xPen, i, ay, i + 1, h - f[0]); ay = (int)(h - f[0]);
                        e.Graphics.DrawLine(yPen, i, gy, i + 1, h - f[1]); gy = (int)(h - f[1]);
                        e.Graphics.DrawLine(zPen, i, my, i + 1, h - f[2]); my = (int)(h - f[2]);
                    }
                }
                return;
            }

            var axis = dgData.SelectedRows[0].Tag as Axes;
            if (axis != null)
            {
                e.Graphics.Clear(Color.Black);
                grav.Draw(e.Graphics, cam);
                axis.Draw(e.Graphics, cam);
            }
            */
        }

        //private void chkKalman_CheckedChanged(object sender, EventArgs e)
        //{
        //    var group = AccelRow.Tag as KalmanGroup;
        //    group.Enabled = chkKalman.Checked;
        //    group = GyroRow.Tag as KalmanGroup;
        //    group.Enabled = chkKalman.Checked;
        //    group = MagRow.Tag as KalmanGroup;
        //    group.Enabled = chkKalman.Checked;
        //}

        //private void numProcNoise_ValueChanged(object sender, EventArgs e)
        //{
        //    SetKalmanParams();
        //}

        //private void SetKalmanParams()
        //{
        //    var group = AccelRow.Tag as KalmanGroup;
        //    group.SetParams((float)numProcNoise.Value, (float)numSensNoise.Value);
        //    group = GyroRow.Tag as KalmanGroup;
        //    group.SetParams((float)numProcNoise.Value, (float)numSensNoise.Value);
        //    group = MagRow.Tag as KalmanGroup;
        //    group.SetParams((float)numProcNoise.Value, (float)numSensNoise.Value);
        //}

        //private void numSensNoise_ValueChanged(object sender, EventArgs e)
        //{
        //    SetKalmanParams();
        //}

      

        private void MonitorForm_FormClosed(object sender, FormClosedEventArgs e)
        {
        }
    }
}

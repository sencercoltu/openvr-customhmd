using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using static monitor_customhmd.UsbPacketDefs;
using System.Globalization;

namespace monitor_customhmd
{    
    public partial class TrackedDevice : UserControl
    {
        public class MinMaxData
        {
            public class Data
            {
                public float[] Accel = new float[3];
                public float[] Gyro = new float[3];
                public float[] Mag = new float[3];
            }
            public Data Max = new Data();

            public Data Min = new Data();

            public Data Med = new Data();



            public void Init()
            {
                for (var i = 0; i < 3; i++)
                {
                    Max.Accel[i] = float.MinValue;
                    Min.Accel[i] = float.MaxValue;
                    Max.Gyro[i] = 0;
                    Min.Gyro[i] = 0;
                    Max.Mag[i] = float.MinValue;
                    Min.Mag[i] = float.MaxValue;
                }
            }
        }

        private MinMaxData MinMax = new MinMaxData();

        private int IncomingPackets;
        private int OutgoingPackets;
        private ushort LastSequence;
        private int MissedPackets;

        private int CommandPackets;
        private int TriggerPackets;
        private int RotationPackets;
        private int PositionPackets;

        private readonly byte DeviceType;
        private DateTime VibrateEnd = DateTime.MinValue;

        public TrackedDevice(byte type)
        {
            MinMax.Init();
            DeviceType = type;
            InitializeComponent();
            foreach (var item in Enum.GetValues(typeof(RawModes)))
                cmbSwitchState.Items.Add(item);
            dgProperties.Rows.Add("Incoming Packets", 0);
            dgProperties.Rows.Add("Outgoing Packets", 0);
            dgProperties.Rows.Add("Missed Packets", 0);
            dgProperties.Rows.Add("Last Sequence", 0);
            dgProperties.Rows.Add("Rotation", 0);
            dgProperties.Rows.Add("Trigger", 0);
            dgProperties.Rows.Add("Command", 0);
            dgProperties.Rows.Add("Position", 0);

            dgQuaternion.Rows.Add("W", 0.0f);
            dgQuaternion.Rows.Add("X", 0.0f);
            dgQuaternion.Rows.Add("Y", 0.0f);
            dgQuaternion.Rows.Add("Z", 0.0f);

            dgButtons.Rows.Add("Analog 0", 0.0f);
            dgButtons.Rows.Add("Analog 1", 0.0f);
            dgButtons.Rows.Add("Digital 0", "OFF");
            dgButtons.Rows.Add("Digital 1", "OFF");
            dgButtons.Rows.Add("Digital 2", "OFF");
            dgButtons.Rows.Add("Digital 3", "OFF");
            dgButtons.Rows.Add("Digital 4", "OFF");
            dgButtons.Rows.Add("Digital 5", "OFF");
            dgButtons.Rows.Add("Digital 6", "OFF");
            dgButtons.Rows.Add("Digital 7", "OFF");

            dgRaw.Rows.Add("Accel", "", "", "", "");
            dgRaw.Rows.Add("", "Min", "", "", "");
            dgRaw.Rows.Add("", "Max", "", "", "");
            dgRaw.Rows.Add("", "Med", "", "", "");
            dgRaw.Rows.Add("Gyro", "", "", "", "");
            dgRaw.Rows.Add("", "Min", "", "", "");
            dgRaw.Rows.Add("", "Max", "", "", "");
            dgRaw.Rows.Add("", "Med", "", "", "");
            dgRaw.Rows.Add("Mag", "", "", "", "");
            dgRaw.Rows.Add("", "Min", "", "", "");
            dgRaw.Rows.Add("", "Max", "", "", "");
            dgRaw.Rows.Add("", "Med", "", "", "");


        }

        public void ProcessPacket(PacketDirection direction, ref USBPacket packet)
        {
            lblVibrate.Text = "Vibrate: " + (DateTime.Now > VibrateEnd ? "OFF" : "ON");
            var typ = (byte)(packet.Header.Type & 0xf0);
            if (direction == PacketDirection.Incoming)
            {
                IncomingPackets++;
                if (packet.Header.Sequence != LastSequence + 1 && LastSequence > 0)
                {
                    MissedPackets += (packet.Header.Sequence - LastSequence);
                    //lblMissed.Text = "Missed Packets: " + MissedPackets;
                    dgProperties.Rows[2].Cells[1].Value = MissedPackets;
                }
                LastSequence = packet.Header.Sequence;
                dgProperties.Rows[3].Cells[1].Value = LastSequence;
                dgProperties.Rows[0].Cells[1].Value = IncomingPackets;
            }
            else if (direction == PacketDirection.Outgoing)
            {
                OutgoingPackets++;
                dgProperties.Rows[1].Cells[1].Value = OutgoingPackets;
            }

            switch (typ)
            {
                case POSITION_DATA:
                    PositionPackets++;
                    dgProperties.Rows[7].Cells[1].Value = PositionPackets;
                    //sonra
                    break;
                case ROTATION_DATA:
                    RotationPackets++;
                    dgProperties.Rows[4].Cells[1].Value = RotationPackets;

                    dgQuaternion.Rows[0].Cells[1].Value = packet.Rotation.w;
                    dgQuaternion.Rows[1].Cells[1].Value = packet.Rotation.x;
                    dgQuaternion.Rows[2].Cells[1].Value = packet.Rotation.y;
                    dgQuaternion.Rows[3].Cells[1].Value = packet.Rotation.z;
                    break;
                case TRIGGER_DATA:
                    TriggerPackets++;
                    dgProperties.Rows[5].Cells[1].Value = TriggerPackets;
                    //var digital = "";
                    //var d = packet.Trigger.Digital;
                    //for (int i = 0; i < sizeof(ushort) * 8; i++)
                    //{
                    //    digital += ((d & 1) == 1) ? "1" : "0";
                    //    d >>= 1;
                    //}

                    dgButtons.Rows[0].Cells[1].Value = packet.Trigger.Analog[0].x;
                    dgButtons.Rows[0].Cells[2].Value = packet.Trigger.Analog[0].y;
                    dgButtons.Rows[1].Cells[1].Value = packet.Trigger.Analog[1].x;
                    dgButtons.Rows[1].Cells[2].Value = packet.Trigger.Analog[1].y;
                    dgButtons.Rows[2].Cells[1].Value = ((packet.Trigger.Digital & BUTTON_0) == BUTTON_0) ? "ON" : "OFF";
                    dgButtons.Rows[3].Cells[1].Value = ((packet.Trigger.Digital & BUTTON_1) == BUTTON_1) ? "ON" : "OFF";
                    dgButtons.Rows[4].Cells[1].Value = ((packet.Trigger.Digital & BUTTON_2) == BUTTON_2) ? "ON" : "OFF";
                    dgButtons.Rows[5].Cells[1].Value = ((packet.Trigger.Digital & BUTTON_3) == BUTTON_3) ? "ON" : "OFF";
                    dgButtons.Rows[6].Cells[1].Value = ((packet.Trigger.Digital & BUTTON_4) == BUTTON_4) ? "ON" : "OFF";
                    dgButtons.Rows[7].Cells[1].Value = ((packet.Trigger.Digital & BUTTON_5) == BUTTON_5) ? "ON" : "OFF";
                    dgButtons.Rows[8].Cells[1].Value = ((packet.Trigger.Digital & BUTTON_6) == BUTTON_6) ? "ON" : "OFF";
                    dgButtons.Rows[9].Cells[1].Value = ((packet.Trigger.Digital & BUTTON_7) == BUTTON_7) ? "ON" : "OFF";


                    break;
                case COMMAND_DATA:
                    CommandPackets++;
                    dgProperties.Rows[6].Cells[1].Value = CommandPackets;
                    switch (packet.Command.Command)
                    {
                        case CMD_VIBRATE:
                            VibrateEnd = DateTime.Now.AddMilliseconds(packet.Command.Vibration.Duration);
                            //outgoing
                            break;
                        case CMD_CALIBRATE:
                            //outgoing - incoming
                            if (direction == PacketDirection.Incoming)
                            {
                                if (packet.Command.Calibration.Command == CALIB_GET)
                                {
                                    minX.Text = packet.Command.Calibration.RawMin[0].ToString();
                                    minY.Text = packet.Command.Calibration.RawMin[1].ToString();
                                    minZ.Text = packet.Command.Calibration.RawMin[2].ToString();
                                    maxX.Text = packet.Command.Calibration.RawMax[0].ToString();
                                    maxY.Text = packet.Command.Calibration.RawMax[1].ToString();
                                    maxZ.Text = packet.Command.Calibration.RawMax[2].ToString();
                                }
                            }
                            break;
                        case CMD_SYNC:
                            //incoming from basestation
                            break;
                        case CMD_RAW_DATA:
                            //incoming
                            for (var i=0; i<3; i++)
                            {
                                MinMax.Med.Accel[i] = (MinMax.Med.Accel[i] * 0.99f) + (packet.Command.Raw.Accel[i] * 0.01f);
                                MinMax.Med.Gyro[i] = (MinMax.Med.Gyro[i] * 0.99f) + (packet.Command.Raw.Gyro[i] * 0.01f);
                                MinMax.Med.Mag[i] = (MinMax.Med.Mag[i] * 0.99f) + (packet.Command.Raw.Mag[i] * 0.01f);

                                if (packet.Command.Raw.Accel[i] > MinMax.Max.Accel[i]) MinMax.Max.Accel[i] = packet.Command.Raw.Accel[i];
                                if (packet.Command.Raw.Accel[i] < MinMax.Min.Accel[i]) MinMax.Min.Accel[i] = packet.Command.Raw.Accel[i];

                                if (packet.Command.Raw.Gyro[i] > MinMax.Max.Gyro[i]) MinMax.Max.Gyro[i] = packet.Command.Raw.Gyro[i];
                                if (packet.Command.Raw.Gyro[i] < MinMax.Min.Gyro[i]) MinMax.Min.Gyro[i] = packet.Command.Raw.Gyro[i];

                                if (packet.Command.Raw.Mag[i] > MinMax.Max.Mag[i]) MinMax.Max.Mag[i] = packet.Command.Raw.Mag[i];
                                if (packet.Command.Raw.Mag[i] < MinMax.Min.Mag[i]) MinMax.Min.Mag[i] = packet.Command.Raw.Mag[i];
                            }

                            dgRaw.Rows[0].Cells[2].Value = packet.Command.Raw.Accel[0];
                            dgRaw.Rows[0].Cells[3].Value = packet.Command.Raw.Accel[1];
                            dgRaw.Rows[0].Cells[4].Value = packet.Command.Raw.Accel[2];
                            dgRaw.Rows[1].Cells[2].Value = MinMax.Min.Accel[0];
                            dgRaw.Rows[1].Cells[3].Value = MinMax.Min.Accel[1];
                            dgRaw.Rows[1].Cells[4].Value = MinMax.Min.Accel[2];
                            dgRaw.Rows[2].Cells[2].Value = MinMax.Max.Accel[0];
                            dgRaw.Rows[2].Cells[3].Value = MinMax.Max.Accel[1];
                            dgRaw.Rows[2].Cells[4].Value = MinMax.Max.Accel[2];
                            dgRaw.Rows[3].Cells[2].Value = MinMax.Med.Accel[0];
                            dgRaw.Rows[3].Cells[3].Value = MinMax.Med.Accel[1];
                            dgRaw.Rows[3].Cells[4].Value = MinMax.Med.Accel[2];


                            dgRaw.Rows[4].Cells[2].Value = packet.Command.Raw.Gyro[0];
                            dgRaw.Rows[4].Cells[3].Value = packet.Command.Raw.Gyro[1];
                            dgRaw.Rows[4].Cells[4].Value = packet.Command.Raw.Gyro[2];
                            dgRaw.Rows[5].Cells[2].Value = MinMax.Min.Gyro[0];
                            dgRaw.Rows[5].Cells[3].Value = MinMax.Min.Gyro[1];
                            dgRaw.Rows[5].Cells[4].Value = MinMax.Min.Gyro[2];
                            dgRaw.Rows[6].Cells[2].Value = MinMax.Max.Gyro[0];
                            dgRaw.Rows[6].Cells[3].Value = MinMax.Max.Gyro[1];
                            dgRaw.Rows[6].Cells[4].Value = MinMax.Max.Gyro[2];
                            dgRaw.Rows[7].Cells[2].Value = MinMax.Med.Gyro[0];
                            dgRaw.Rows[7].Cells[3].Value = MinMax.Med.Gyro[1];
                            dgRaw.Rows[7].Cells[4].Value = MinMax.Med.Gyro[2];


                            dgRaw.Rows[8].Cells[2].Value = packet.Command.Raw.Mag[0];
                            dgRaw.Rows[8].Cells[3].Value = packet.Command.Raw.Mag[1];
                            dgRaw.Rows[8].Cells[4].Value = packet.Command.Raw.Mag[2];
                            dgRaw.Rows[9].Cells[2].Value = MinMax.Min.Mag[0];
                            dgRaw.Rows[9].Cells[3].Value = MinMax.Min.Mag[1];
                            dgRaw.Rows[9].Cells[4].Value = MinMax.Min.Mag[2];
                            dgRaw.Rows[10].Cells[2].Value = MinMax.Max.Mag[0];
                            dgRaw.Rows[10].Cells[3].Value = MinMax.Max.Mag[1];
                            dgRaw.Rows[10].Cells[4].Value = MinMax.Max.Mag[2];
                            dgRaw.Rows[11].Cells[2].Value = MinMax.Med.Mag[0];
                            dgRaw.Rows[11].Cells[3].Value = MinMax.Med.Mag[1];
                            dgRaw.Rows[11].Cells[4].Value = MinMax.Med.Mag[2];

                            break;
                        case CMD_STATUS:
                            //incoming
                            break;
                    }
                    break; ;
            }

            //packetsList.Items.Insert(0, "[" + direction + "/" + packet.Header.Sequence + "/" + typ + "]");
            //if (packetsList.Items.Count > 1000) packetsList.Items.RemoveAt(1000);

        }

        private void cmbSwitchState_DropDownClosed(object sender, EventArgs e)
        {
            if (cmbSwitchState.SelectedIndex == -1) return;
            var rawData = new USBRawData();
            rawData.State = (byte)cmbSwitchState.SelectedIndex;
            var command = USBCommandData.Create(CMD_RAW_DATA, rawData);
            var packet = USBPacket.Create((byte)(COMMAND_DATA | DeviceType), (ushort)(DateTime.Now.Ticks / 1000), command);
            var d = StructToBytes(packet);
            SetPacketCrc(ref d);
            //var data = new byte[USBPacket.Size];
            //Array.Copy(d, 1, data, 0, USBPacket.Size);                           
            lock (MonitorForm.OutgoingPackets)
                MonitorForm.OutgoingPackets.Enqueue(d);
            IncomingPackets = 0;
            OutgoingPackets = 0;
            LastSequence = 0;
            MissedPackets = 0;
            MinMax.Init();

        }

        private void btnGetCalib_Click(object sender, EventArgs e)
        {
            if (cmbSensor.SelectedIndex < 1) return;
            minX.Text = ""; maxX.Text = ""; minY.Text = ""; maxY.Text = ""; minZ.Text = ""; maxZ.Text = "";
            var calibData = new USBCalibrationData();
            calibData.Init();
            calibData.Command = (byte)CALIB_GET;
            calibData.Sensor = (byte)cmbSensor.SelectedIndex;
            var command = USBCommandData.Create(CMD_CALIBRATE, calibData);
            var packet = USBPacket.Create((byte)(COMMAND_DATA | DeviceType), (ushort)(DateTime.Now.Ticks / 1000), command);
            var d = StructToBytes(packet);
            SetPacketCrc(ref d);
            //var data = new byte[USBPacket.Size];
            //Array.Copy(d, 1, data, 0, USBPacket.Size);                           
            lock (MonitorForm.OutgoingPackets)
                MonitorForm.OutgoingPackets.Enqueue(d);
        }

        private void btnSetCalib_Click(object sender, EventArgs e)
        {
            if (cmbSensor.SelectedIndex < 1) return;
            var calibData = new USBCalibrationData();
            calibData.Init();
            calibData.Command = (byte)CALIB_SET;
            calibData.Sensor = (byte)cmbSensor.SelectedIndex;
            short.TryParse(minX.Text, out calibData.RawMin[0]);
            short.TryParse(minY.Text, out calibData.RawMin[1]);
            short.TryParse(minZ.Text, out calibData.RawMin[2]);
            short.TryParse(maxX.Text, out calibData.RawMax[0]);
            short.TryParse(maxY.Text, out calibData.RawMax[1]);
            short.TryParse(maxZ.Text, out calibData.RawMax[2]);
            var command = USBCommandData.Create(CMD_CALIBRATE, calibData);
            var packet = USBPacket.Create((byte)(COMMAND_DATA | DeviceType), (ushort)(DateTime.Now.Ticks / 1000), command);
            var d = StructToBytes(packet);
            SetPacketCrc(ref d);
            //var data = new byte[USBPacket.Size];
            //Array.Copy(d, 1, data, 0, USBPacket.Size);                           
            lock (MonitorForm.OutgoingPackets)
                MonitorForm.OutgoingPackets.Enqueue(d);
        }

        private void TrackedDevice_Load(object sender, EventArgs e)
        {

        }
    }
}

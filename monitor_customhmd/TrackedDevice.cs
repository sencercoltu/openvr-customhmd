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
        private int IncomingPackets;
        private int OutgoingPackets;
        private ushort LastSequence;
        private int MissedPackets;
        private readonly byte DeviceType;


        public TrackedDevice(byte type)
        {
            DeviceType = type;
            InitializeComponent();
            foreach (var item in Enum.GetValues(typeof(RawModes)))
                cmbSwitchState.Items.Add(item);
        }

        public void ProcessPacket(PacketDirection direction, ref USBPacket packet)
        {
            var typ = (byte)(packet.Header.Type & 0xf0);
            if (direction == PacketDirection.Incoming)
            {
                IncomingPackets++;
                if (packet.Header.Sequence != LastSequence + 1 && LastSequence > 0)
                {
                    MissedPackets += (packet.Header.Sequence - LastSequence);
                    lblMissed.Text = "Missed Packets: " + MissedPackets;
                }
                LastSequence = packet.Header.Sequence;
                lblLastSequence.Text = "Last Sequence: " + LastSequence;
                lblIncoming.Text = "Incoming Packets: " + IncomingPackets;
            }
            else if (direction == PacketDirection.Outgoing)
            {
                OutgoingPackets++;
                lblOutgoing.Text = "Outgoing Packets: " + OutgoingPackets;
            }
            
            switch(typ)
            {
                case POSITION_DATA:
                    //sonra
                    break;
                case ROTATION_DATA:
                    lblQuat.Text = "Quaternions [QW: " + packet.Rotation.w.ToString("N4", CultureInfo.InvariantCulture) + " / " +
                                   "QX: " + packet.Rotation.x.ToString("N4", CultureInfo.InvariantCulture) + " / " +
                                   "QY: " + packet.Rotation.y.ToString("N4", CultureInfo.InvariantCulture) + " / " +
                                   "QZ: " + packet.Rotation.z.ToString("N4", CultureInfo.InvariantCulture) + "]";
                    break;
                case TRIGGER_DATA:
                    var digital = "";
                    var d = packet.Trigger.Digital;
                    for (int i=0; i<sizeof(ushort) * 8;i++)
                    {
                        digital += ((d & 1) == 1)? "1":"0";                            
                        d >>= 1;
                    }
                    lblTriggers.Text = "Triggers [DT: " + digital + " / " +
                                       "A0: (" + packet.Trigger.Analog[0].x.ToString("N1", CultureInfo.InvariantCulture) + ";" + packet.Trigger.Analog[0].y.ToString("N1", CultureInfo.InvariantCulture) + ") / " +
                                       "A1: (" + packet.Trigger.Analog[1].x.ToString("N1", CultureInfo.InvariantCulture) + ";" + packet.Trigger.Analog[1].y.ToString("N1", CultureInfo.InvariantCulture) + ") / " +
                                       "A2: (" + packet.Trigger.Analog[2].x.ToString("N1", CultureInfo.InvariantCulture) + ";" + packet.Trigger.Analog[2].y.ToString("N1", CultureInfo.InvariantCulture) + ")]";
                    break;
                case COMMAND_DATA:
                    switch(packet.Command.Command)
                    {
                        case CMD_VIBRATE:
                            //outgoing
                            break;
                        case CMD_CALIBRATE:
                            //outgoing
                            break;
                        case CMD_SYNC:
                            //incoming from basestation
                            break;
                        case CMD_RAW_DATA:
                            //incoming
                            break;
                        case CMD_STATUS:
                            //incoming
                            break;
                    }
                    break; ;
    }

    packetsList.Items.Insert(0, "[" + direction  + "/" + packet.Header.Sequence + "/" + typ + "]");
            if (packetsList.Items.Count > 1000) packetsList.Items.RemoveAt(1000);

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
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static monitor_customhmd.UsbPacketDefs;

namespace monitor_customhmd.DriverComm
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
    public struct OpenTrackPacket
    {
        private static double DegToRad = (3.14159265358979323846 / 180.0);        

        public static int Size = Marshal.SizeOf(typeof(OpenTrackPacket));
        public double X;
        public double Y;
        public double Z;
        public double Yaw;
        public double Pitch;
        public double Roll;

        public void Init(out USBRotationData rotation, out USBPositionData position)
        {
            rotation = new USBRotationData();

            //Convert yaw, pitch, roll to quaternion
            var t0 = Math.Cos(Yaw * DegToRad * 0.5);
            var t1 = Math.Sin(Yaw * DegToRad * 0.5);
            var t2 = Math.Cos(Roll * DegToRad * 0.5);
            var t3 = Math.Sin(Roll * DegToRad * 0.5);
            var t4 = Math.Cos(Pitch * DegToRad * 0.5);
            var t5 = Math.Sin(Pitch * DegToRad * 0.5);

            //Set head tracking rotation
            rotation.w = (float)(t0 * t2 * t4 + t1 * t3 * t5);
            rotation.x = (float)(t0 * t3 * t4 - t1 * t2 * t5);
            rotation.y = (float)(t0 * t2 * t5 + t1 * t3 * t4);
            rotation.z = (float)(t1 * t2 * t4 - t0 * t3 * t5);

            position = new USBPositionData();
            position.Position = new float[3];
            position.Position[0] = (float)X;
            position.Position[1] = (float)Y;
            position.Position[2] = (float)Z;
        }
    }
}
